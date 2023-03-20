#include "broadcast-strategy.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"

#include <ndn-cxx/lp/tags.hpp>

namespace nfd {
namespace fw {
namespace bcvalc {

NFD_REGISTER_STRATEGY(BroadcastStrategy);

NFD_LOG_INIT(BroadcastStrategy);


BroadcastStrategy::BroadcastStrategy(Forwarder& forwarder, const Name& name)
    : Strategy(forwarder)
    , m_measurements(getMeasurements())
    , m_measurementsCreations(0)
    , m_measurementsLookUps(0)
    , m_commValFace(forwarder.getCommValFace())
{
    NFD_LOG_DEBUG("making broadcast strategy");
    ParsedInstanceName parsed = parseInstanceName(name);
    if (!parsed.parameters.empty()) {
        BOOST_THROW_EXCEPTION(std::invalid_argument("BroadcastStrategy does not accept parameters"));
    }
    if (parsed.version && *parsed.version != getStrategyName()[-1].toVersion()) {
        BOOST_THROW_EXCEPTION(std::invalid_argument(
        "BroadcastStrategy does not support version " + to_string(*parsed.version)));
    }
    this->setInstanceName(makeInstanceName(name, getStrategyName()));
}

const Name&
BroadcastStrategy::getStrategyName()
{
    static Name strategyName("ndn:/localhost/nfd/strategy/broadcast/%FD%04");
    return strategyName;
}

/**
 * after a CS MISS, when a Interest arrives without forwardingHint it ends up here
 * upStream faces are given by the FIB
 * the node can be:
 * a consumer (locally gen Interest)
 * a relay
 * a producer (fib has a appFace)
 * it can serve:
 */
void
BroadcastStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
    NFD_LOG_DEBUG("receiving interest CS MISS in broadcast strategy");

    Face* outFace = getOutFaceFromFib(pitEntry, inFace, interest);
    shared_ptr<lp::NextHopFaceIdTag> nextHopTag = interest.getTag<lp::NextHopFaceIdTag>();
    Face* nextHopFace = nullptr;
    
    if(nextHopTag != nullptr) {
        nextHopFace = this->getFaceTable().get(*nextHopTag);
    }

    if(outFace == nullptr && nextHopFace != nullptr) {
        outFace = nextHopFace;
        NFD_LOG_DEBUG("nextHopFace is now the outFace");    
    }
    
    if(outFace == nullptr && nextHopFace == nullptr) {
        this->rejectPendingInterest(pitEntry);
        NFD_LOG_DEBUG("no Face selected");
        return;
    }

    if(outFace != nullptr && outFace->isGeoFace()) {
        if(nextHopFace != nullptr) {
            fib::Entry* fibEntry = this->getFib().insert(interest.getName().getPrefix(-1)).first;
            fibEntry->addOrUpdateNextHop(*nextHopFace, 0, 1);
            outFace = nextHopFace;
            NFD_LOG_DEBUG("Updated FIB NextHop = Face " << outFace->getId());
        }
        if(outFace->isGeoFace() && outFace->getId() > 256) {
            setRouteInCheck(this->lookupFib(*pitEntry), outFace);
        }
    }

    this->sendInterest(pitEntry, *outFace, interest);
    NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " pitEntry-to=" << outFace->getId());

}

/**
 * node can be a consumer or a relay
 * Interest match with CS generates Data and ends up here
 * if the inFace is a geoface we must change de default behavior
 * and get the outFace from fib
 * otherwise the behaviour stays the default
 */
void
BroadcastStrategy::afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
                       const Face& inFace, const Data& data)
{
    NFD_LOG_DEBUG(__func__);
    if(inFace.isGeoFace()) {
        const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
        const fib::NextHopList& nexthops = fibEntry.getNextHops();
        for(const auto& nexthop : nexthops) {
            Face& producerFace = nexthop.getFace();
            if(producerFace.isGeoFace()){ // getting origin of this data
                NFD_LOG_DEBUG("afterContentStoreHit GeoFace pitEntry=" << pitEntry->getName() <<
                " inFace=" << inFace.getId() << " producerFace=" << producerFace.getId() <<
                " data=" << data.getName());
                data.setTag(make_shared<lp::CongestionMarkTag>(producerFace.getId()));
                this->sendDataToVal(pitEntry, data, inFace, &pitEntry->getNonceList(), false);
                break;
            }
        }
        NFD_LOG_DEBUG("End of nexthops iteration");
    } else {  // interest satisfied locally
        NFD_LOG_DEBUG("afterContentStoreHit pitEntry=" << pitEntry->getName() <<
                " inFace=" << inFace.getId() << " data=" << data.getName());
        this->sendData(pitEntry, data, inFace);
    }
}

void
BroadcastStrategy::afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                   const Face& inFace, const Data& data)
{
    NFD_LOG_DEBUG("afterReceiveData FROM BROADCAST pitEntry=" << pitEntry->getName() <<
                " inFace=" << inFace.getId() << " data=" << data.getName());

    this->beforeSatisfyInterest(pitEntry, inFace, data);


    std::set<Face*> pendingDownstreams;
    auto now = time::steady_clock::now();

    // check if the data pkt was produced locally
    bool isProducer = true;
    // adding route to FIB
    if(inFace.isGeoFace()) {
        fib::Entry* fibEntry = this->getFib().insert(data.getName().getPrefix(-1)).first;
        fibEntry->addOrUpdateNextHop(const_cast<Face&>(inFace), 0, 1);
        isProducer = false;
        // removing route expryring events
        for(const pit::OutRecord& outRecord : pitEntry->getOutRecords()) {
            if(outRecord.getFace().getId() == inFace.getId()) {
                removeRouteFromCheck(*fibEntry, inFace);
            }
        }
    }
    

    
    // remember pending downstreams
    // one geoface and all the other faces
    for(const pit::InRecord& inRecord : pitEntry->getInRecords()) {
        if(inRecord.getExpiry() > now) {
            pendingDownstreams.insert(&inRecord.getFace());
        }
    }

    bool sentToGeoFace = false;
    std::vector<uint32_t> *nonceList = nullptr;
    for(const Face* pendingDownstream : pendingDownstreams) {
        if(pendingDownstream->isGeoFace() && !sentToGeoFace) {
            this->sendDataToVal(pitEntry, data, *pendingDownstream, &pitEntry->getNonceList(), isProducer);
            sentToGeoFace = true;
        } else if(!pendingDownstream->isGeoFace()) {
            if(inFace.isGeoFace() && pendingDownstream->getLocalUri().toString() == "appFace://") {
                nonceList = &pitEntry->getNonceList();
            }
            this->sendData(pitEntry, data, *pendingDownstream);
        } else {
            pitEntry->deleteInRecord(*pendingDownstream);
        }
    }
    // if is app face notify valForwareder so it can clean any pending imp ACK
    if(nonceList != nullptr) {
        NFD_LOG_DEBUG("SENDING 500!!!!!!!!!!");
        data.setTag(make_shared<lp::CongestionMarkTag>(500));
        m_commValFace->sendDataToVal(data, nonceList, false);
        //const_cast<Face&>(inFace).sendDataToVal(data, nonceList, false);
    }
}

// if there is an appFace returns the appFace other wise returns a geoFace
Face*
BroadcastStrategy::getOutFaceFromFib(const shared_ptr<pit::Entry>& pitEntry, const Face& inFace, const Interest& interest)
{
    const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
    const fib::NextHopList& nexthops = fibEntry.getNextHops();
    bool isSet = false;
    Face* outFace = nullptr;

    for (const auto& nexthop : nexthops) {
        Face& candidateFace = nexthop.getFace();
        
        // only appfaces and geoface
        if(!candidateFace.isValNetFace() && !wouldViolateScope(inFace, interest, candidateFace)) {
            // producer
            if(candidateFace.getLocalUri().toString() == "appFace://") { // is appFace
                outFace = &candidateFace;
                break;   // if one appFace is found ends the search (WHY?) (R)it found the producer 
            }
            // consumer or relay
            if(candidateFace.isGeoFace() && !isSet) {
                outFace = &candidateFace;
                isSet = true;
            }
        }
    }
    return outFace;
}

void
BroadcastStrategy::setRouteInCheck(const fib::Entry& fibEntry, Face* outFace)
{
    NFD_LOG_DEBUG(__func__);

    // get or create the measuremente entry
    RouteAvailabilityInfo* routeInfo = m_measurements.getOrCreateRouteAvailabilityInfo(fibEntry);
    // make the evente for the deletion of this route
    const FaceId faceId = outFace->getId();
    if(routeInfo != nullptr  && routeInfo->getRouteExperationId(faceId) == nullptr) {
        NFD_LOG_DEBUG("setting expiration timer on " << routeInfo->getName().toUri() <<
        " faceid: " << faceId);
        routeInfo->addRouteExpiration(faceId,
        ::nfd::scheduler::schedule(300_ms, [&fibEntry, outFace, this] { disableRouteCallBack(fibEntry, outFace); }));
    }
}

void
BroadcastStrategy::removeRouteFromCheck(const fib::Entry& fibEntry, const Face& inFace)
{
    NFD_LOG_DEBUG(__func__);

    RouteAvailabilityInfo* routeInfo = m_measurements.getRouteAvailabilityInfo(fibEntry.getPrefix());
    if(routeInfo != nullptr) {
          scheduler::cancel(routeInfo->getRouteExperationId(inFace.getId()));
          NFD_LOG_DEBUG("Route validated: " << fibEntry.getPrefix().toUri() <<
          " faceId: " << inFace.getId());
    }
}

void
BroadcastStrategy::disableRouteCallBack(const fib::Entry& fibEntry, Face* face)
{
    NFD_LOG_DEBUG(__func__);

    // remove route from BcValCMeasurements::RouteExpirationTable
    RouteAvailabilityInfo* routeInfo = m_measurements.getRouteAvailabilityInfo(fibEntry.getPrefix());
    if(routeInfo != nullptr && routeInfo->removeRouteExpiration(face->getId())) {
        // remove from fib
        const_cast<fib::Entry&>(fibEntry).removeNextHopByFace(*face);
        NFD_LOG_DEBUG("ROUTE DELETION " << fibEntry.getPrefix().toUri() << "@ geoFace: "
        << face->getId());
        if(!fibEntry.hasNextHops()) {
            NFD_LOG_DEBUG("removing fib entry");
            m_measurements.removeEntry(fibEntry);
            this->getFib().erase(fibEntry);
        }
    } 
}

}
} // namespace fw
} // namespace nfd
