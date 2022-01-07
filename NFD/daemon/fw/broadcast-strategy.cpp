#include "broadcast-strategy.hpp"
#include "algorithm.hpp"
#include "core/logger.hpp"

#include <ndn-cxx/lp/tags.hpp>

namespace nfd {
namespace fw {

NFD_REGISTER_STRATEGY(BroadcastStrategy);

NFD_LOG_INIT(BroadcastStrategy);


BroadcastStrategy::BroadcastStrategy(Forwarder& forwarder, const Name& name)
    : Strategy(forwarder)
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
    static Name strategyName("ndn:/localhost/nfd/strategy/broadcast/%FD%03");
    return strategyName;
}

/**
 * after a CS MISS, when a Interest arrives without forwardingHint it ends up here
 * upStream faces are given by the FIB
 * the node can be:
 * a consumer (locally gen Interest)
 * a relay
 * a producer (fib as a appFace)
 * it can serve:
 */
void
BroadcastStrategy::afterReceiveInterest(const Face& inFace, const Interest& interest,
                                        const shared_ptr<pit::Entry>& pitEntry)
{
    NFD_LOG_DEBUG("receiving interest CS MISS in broadcast strategy");
    const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
    const fib::NextHopList& nexthops = fibEntry.getNextHops();

    bool isSet = false;
    Face* outFace = nullptr;

    for (const auto& nexthop : nexthops) {
        Face& candidateFace = nexthop.getFace();
        
        // only appfaces and geoface
        if(!candidateFace.isValNetFace() && !wouldViolateScope(inFace, interest, candidateFace)) {
            if(candidateFace.getLocalUri().toString() == "appFace://") { // is appFace
                outFace = &candidateFace;
                break;   // if one appFace is found ends the search
            }
            if(candidateFace.isGeoFace() && !isSet) {
                outFace = &candidateFace;
                isSet = true;
            }
        }
    }

    if(outFace == nullptr) {
        this->rejectPendingInterest(pitEntry);
        NFD_LOG_DEBUG("no Face selected");
    } else {
        this->sendInterest(pitEntry, *outFace, interest);
        NFD_LOG_DEBUG(interest << " from=" << inFace.getId()
                           << " pitEntry-to=" << outFace->getId());
    }
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
    if(inFace.isGeoFace()) {
        const fib::Entry& fibEntry = this->lookupFib(*pitEntry);
        const fib::NextHopList& nexthops = fibEntry.getNextHops();
        for(const auto& nexthop : nexthops) {
            Face& outFace = nexthop.getFace();
            if(outFace.isGeoFace()){ // getting origin of this data
                NFD_LOG_DEBUG("afterContentStoreHit GeoFace pitEntry=" << pitEntry->getName() <<
                " inFace=" << inFace.getId() << " outFace=" << outFace.getId() <<
                " data=" << data.getName());
                this->sendDataToVal(pitEntry, data, outFace, &pitEntry->getNonceList(), false);
                break;
            }
        }
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
    }
    
    // remember pending downstreams
    // one geoface and all the other faces
    for(const pit::InRecord& inRecord : pitEntry->getInRecords()) {
        if(inRecord.getExpiry() > now) {
            pendingDownstreams.insert(&inRecord.getFace());
        }
    }

    bool sentToGeoFace = false;
    for(const Face* pendingDownstream : pendingDownstreams) {
        if(pendingDownstream->isGeoFace() && !sentToGeoFace) {
            this->sendDataToVal(pitEntry, data, *pendingDownstream, &pitEntry->getNonceList(), isProducer);
            sentToGeoFace = true;
        } else if(!pendingDownstream->isGeoFace()) {
            std::vector<uint32_t> *nonceList = nullptr;
            if(inFace.isGeoFace() && pendingDownstream->getLocalUri().toString() == "appFace://") {
                nonceList = &pitEntry->getNonceList();
            }
            this->sendData(pitEntry, data, *pendingDownstream);
            // if is app face notify valForwareder so it can clean any pending imp ACK
            if(nonceList != nullptr) {
                data.setTag(make_shared<lp::CongestionMarkTag>(500));
                const_cast<Face&>(inFace).sendDataToVal(data, nonceList, false);
            }
        } else {
            pitEntry->deleteInRecord(*pendingDownstream);
        }
    }
}

} // namespace fw
} // namespace nfd