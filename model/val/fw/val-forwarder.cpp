/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-forwarder.hpp"
#include "val-distances-strategy.hpp"
#include "ns3/ndnSIM/model/ndn-l3-protocol.hpp"
#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
#include "ns3/log.h"

#include <ndn-cxx/lp/tags.hpp>



NS_LOG_COMPONENT_DEFINE("ndn.val.ValForwarder");

namespace ns3 {
namespace ndn {
namespace val {

namespace tlv {
    using namespace ::ndn::tlv;
} // namespace tlv

using ::ndn::Interest;
using ::ndn::Data;


ValForwarder::ValForwarder(L3Protocol& l3P)
    : m_l3P(&l3P)
    , m_strategy(new ValDistancesStrategy(*this))
    , m_geofaceFactory(*this)
    , m_invalidIN(0)
{
    NS_LOG_DEBUG("Creating VAL"); 
    m_faceTable = &(m_l3P->getForwarder()->getFaceTable());
    m_faceTable->afterValFaceAdd.connect([this] (Face& face) {
      NS_LOG_DEBUG("Adding ValNetFace? "<< std::boolalpha << face.isValNetFace());
      addToNetworkFaceList(face);
      m_strategy->setMobilityModel(dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport())->GetNetDevice()->GetNode()->GetObject<WaypointMobilityModel>());
      face.afterReceiveValPkt.connect(
        [this, &face] (const ValPacket& valP) {
          onReceivedValPacket(face, valP);
        });
    });

    m_faceTable->beforeValFaceRemove.connect([this] (Face& face) {
        cleanupOnFaceRemoval(face);
    });
    // TODO: it is possible to know if a geoface have been removed by NFD,
    // beforeFaceRemove check this

    // creating flood face, adding to FIB as default route and adding to f2a table
    auto floodFace = m_geofaceFactory.makeGeoface();
    m_l3P->addFace(floodFace);  // adding face to face table
    nfd::fib::Entry* entry = m_l3P->getForwarder()->getFib().insert("/").first;
    entry->addOrUpdateNextHop(*floodFace, 0, 1);
    f2a::Entry floodFaceEntry(floodFace->getId(), "0");  // this can only be made after adding face to face table
    m_f2a.addEntry(floodFaceEntry);
}

ValForwarder::~ValForwarder()
{
  delete m_strategy;
}

void
ValForwarder::onReceivedValPacket(const Face& face, const ValPacket& valP)
{
  NS_LOG_DEBUG(__func__);
  // Is a broadcast from a node in a better possition or a Implicit ACK ?
  if(!preProcessingValPacket(face, valP)) {
    NS_LOG_DEBUG("Packet Dropped");
    return;
  }
  try {
    switch (valP.isSet()) {
      case ValPacket::INTEREST_SET:
      { // braces to avoid "transfer of control bypasses" problem
        // forwarding expects Interest to be created with make_shared
        auto interest = make_shared<Interest>(valP.getInterest());
        processInterestFromNetwork(face, valP.getValHeader(), *interest);
        break;
      }
      case ValPacket::DATA_SET:
      {
        // forwarding expects Interest to be created with make_shared
        auto data = make_shared<Data>(valP.getData());
        processDataFromNetwork(face, valP.getValHeader(), *data);
        break;
      }
      default:
        m_invalidIN++;
        NS_LOG_DEBUG("unrecognized network-layer packet TLV-TYPE " << valP.isSet() << ": DROP");
        return;
    }
  }
  catch (const tlv::Error& e) {
    m_invalidIN++;
    NS_LOG_DEBUG("packet parse error (" << e.what() << "): DROP");
  }
}

bool
ValForwarder::preProcessingValPacket(const Face& face, const ValPacket& valP)
{
  // the first thing to do is to check it agains PFT and see if it is a
  // transmission of a node in a better position or an Implicit ACK
  // the PFT - Pending Forwarding Table entries identify both scenarios
  auto pair = m_pft.findMatch(valP);
  if(pair.first) {  // match found
    // checking the pft entry state
    if(pair.second->getState() == pft::Entry::WAITING_FORWARDING) { // a node in a better position made a broadcast
      if(pair.second->getValPacket().getValHeader().getHopC() == valP.getValHeader().getHopC()) {
        ::nfd::scheduler::cancel(pair.second->getTimerId()); // cancelling the forwarding timer
        NS_LOG_DEBUG("Forwarding cancel due to broadcast of a node in a better position");
        pair.second->changeStateToWaintingImpAck(); // or removing???
        if(pair.second->getValPacket().isSet() == ValPacket::DATA_SET)
          m_pft.removeEntry(valP);
      } else {
        NS_LOG_DEBUG("Forwarding not cancel due to difference in the hop count. Received: " <<
                      std::to_string(valP.getValHeader().getHopC()) << " in PFT: " <<
                      std::to_string(pair.second->getValPacket().getValHeader().getHopC()));
      }
    } else
    if(pair.second->getState() == pft::Entry::WAITING_IMPLICIT_ACK) {
      // for implicit ACK the hop count is less one than the one in PFT
      if(pair.second->getValPacket().getValHeader().getHopC() - 1 == valP.getValHeader().getHopC() || 
          (pair.second->getValPacket().isSet() == ValPacket::DATA_SET && pair.second->getValPacket().getValHeader().getHopC() == valP.getValHeader().getHopC())) {
        ::nfd::scheduler::cancel(pair.second->getTimerId()); // cancelling the forwarding timer
        NS_LOG_DEBUG("Implicit ACK received, retransmission canceled");
        if(pair.second->getValPacket().isSet() == ValPacket::DATA_SET)
          m_pft.removeEntry(valP);
      } else {
        NS_LOG_DEBUG("Implicit ACK not cancel due to difference in the hop count. Received: " <<
                      std::to_string(valP.getValHeader().getHopC()) << " in PFT: " <<
                      std::to_string(pair.second->getValPacket().getValHeader().getHopC()));
      }
    }
  }
  return !pair.first; // true if no match is found
}

bool
ValForwarder::isEscapedPacket(const ValHeader& valH)
{
  // this compares the destination area of the interest, the position of the previous hop
  // and the current node position.
  // if the previous node position is inside the destination area of the interest but
  // the current position of the node is outside that same area, the interest is 
  // dropped
  return false;
}

void
ValForwarder::processInterestFromNetwork(const Face& face, const ValHeader& valH, const Interest& interest)
{
  NS_LOG_DEBUG(__func__);
  // here we process an Interest packet that came from the network
  if(isEscapedPacket(valH))
    return;  // drop packet
  // add ifnt entry
  ifnt::Entry entry(valH, interest, face.getId());
  if(m_ifnt.addEntry(entry)) {  // added - no equal match found
    // get or create geoface
    auto geoFacesIds = makeGeoFaceFromValHeader(valH);
    nfd::Face* geoface_SA = m_faceTable->get(geoFacesIds.first); // get geoface
    if(valH.getDA() != "0")
      interest.setTag(make_shared<lp::NextHopFaceIdTag>(geoFacesIds.second));
    if(geoface_SA != nullptr) {
      geoface_SA->sendInterestToForwarder(std::move(interest)); // send via geoface
    } else {
      NS_LOG_DEBUG("Geoface is nullptr!!");
    }
  } else { // not added already has one entry with the same information
    NS_LOG_DEBUG("Already treating this interest: dropping");
  }
}

void
ValForwarder::processDataFromNetwork(const Face& face, const ValHeader& valH, const Data& data)
{
  NS_LOG_DEBUG(__func__);
  dfnt::Entry entry(valH, data, face.getId());
  m_dfnt.addEntry(entry);  // adding to data from network table
  auto geoFacesIds = makeGeoFaceFromValHeader(valH);
  nfd::Face* geoface_SA = m_faceTable->get(geoFacesIds.first); // get geoface
  if(geoface_SA != nullptr) {
    geoface_SA->sendDataToForwarder(std::move(data)); // send via geoface
  } else {
    NS_LOG_DEBUG("Geoface is nullptr!!");
  }
}

void
ValForwarder::reveiceInterest(const nfd::Face *inGeoface, const Interest& interest)
{
  NS_LOG_DEBUG(__func__);
  auto pair = m_ifnt.findMatchByNonce(interest.getNonce());
  if(pair.first) { // relay node
    NS_LOG_DEBUG("Interest from network");
    m_strategy->afterIfntHit(inGeoface->getId(), pair.second, interest);
  } else {  // generated locally
    NS_LOG_DEBUG("Interest generated locally");
    m_strategy->afterIfntMiss(inGeoface->getId(), interest);
  }
}

void
ValForwarder::reveiceData(const nfd::Face *inGeoface, const Data& data, std::vector<uint32_t> *nonceList, bool isProducer)
{
  NS_LOG_DEBUG(__func__);
  if(data.getCongestionMark() == 501) {
    auto to_remove = m_dfnt.findMatch(data.getSignature());
    if(to_remove.first){
      m_dfnt.removeEntry(*to_remove.second);
      if(isProducer) {// also remove geoface
        //m_f2a.removeByFaceId(inGeoface->getId());
      }
    }
    return;
  }
  // get ifnt entries
  ifnt::ListMatchResult ifntEntriesList = m_ifnt.findMatchByNonceList(nonceList);
  m_ifnt.removeEntriesByNonceList(nonceList);
  auto pair = m_dfnt.findMatch(data.getSignature());
  if(pair.first && !isProducer) { // relay node
    NS_LOG_DEBUG("Data from network");
    // check agains PFT for cancelling retransmission of pending Interests
    // Data can serve as an Implicit ACK or even as Interest Forwarding cancelation
    pft::ListMatchResult pendingInterests = m_pft.findMatchByNonceList(nonceList);
    for(auto it = pendingInterests.begin(); it != pendingInterests.end(); it++) {
      ::nfd::scheduler::cancel((*it)->getTimerId());
    }
    m_pft.removeEntriesByMatchList(pendingInterests);
    m_dfnt.removeEntry(*pair.second); // this may cause trouble
    if(data.getCongestionMark() != 500) { // if 500 just clean entries and discards packet
      m_strategy->afterDfntHit(inGeoface->getId(), pair.second, &ifntEntriesList, data);
    } else {
      NS_LOG_DEBUG("Control comamnd: cleaning");
    }
  } else {  // generated locally
    NS_LOG_DEBUG("Data generated locally");
    m_strategy->afterDfntMiss(inGeoface->getId(), data, &ifntEntriesList, isProducer);
    if(pair.first){
      m_dfnt.removeEntry(*pair.second); // this may cause trouble
    }
  }
}

void
ValForwarder::registerOutgoingValPacket(const nfd::FaceId outFaceId, ValPacket& valPkt, time::nanoseconds duration)
{
  // creates pft entry and schedules the forwarding events
  pft::Entry pftEntry(std::move(valPkt), outFaceId);
  auto pair = m_pft.addEntry(pftEntry);
  if(pair.first) {   // created
    /*Face* outFace = m_faceTable->get(outFaceId);
    outFace->sendValPacket(pair.second->getValPacket());
    return;*/
    NS_LOG_DEBUG("Registering OutgoingValPacket, sending in: " << duration);
    pair.second->setTimer(::nfd::scheduler::schedule(duration, [=] { forwardingTimerCallback(pair.second, outFaceId); }));
  } else {    // not created but found one
    NS_LOG_DEBUG("this should not happen!!");
  }
}

void
ValForwarder::forwardingTimerCallback(const std::shared_ptr<pft::Entry>& pftEntry, const nfd::FaceId outFaceId)
{
  if(pftEntry->getState() == pft::Entry::WAITING_FORWARDING) {
    NS_LOG_DEBUG("Fire forwarding timer callback");
    Face* outFace = m_faceTable->get(outFaceId);
    if(outFace->isValNetFace()) {
      outFace->sendValPacket(pftEntry->getValPacket());
      if(pftEntry->getValPacket().isSet() == ValPacket::DATA_SET)
        NS_LOG_DEBUG("DATA Forwarding timer " << pftEntry->getValPacket().getData().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos());
      else
        NS_LOG_DEBUG("INTERST Forwarding timer " << pftEntry->getValPacket().getInterest().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos());
      // setting ImpAck timer
      pftEntry->changeStateToWaintingImpAck();  // changing state
      pftEntry->setTimer(::nfd::scheduler::schedule(pftEntry->getDefaultImpAckTimerDuration(), [=] { impAckTimerCallback(pftEntry, outFaceId); }));
      // Data Last hop does not receive ImpACK
      if(pftEntry->getValPacket().isSet() == ValPacket::DATA_SET && pftEntry->getValPacket().getValHeader().getHopC() <= 2) {
        ::nfd::scheduler::cancel(pftEntry->getTimerId());
        m_pft.removeEntry(pftEntry->getValPacket());
      }
    } else {
      NS_LOG_DEBUG("NOT ValNetFace: this should never happen!!");
    }
  }
}

void
ValForwarder::impAckTimerCallback(const std::shared_ptr<pft::Entry>& pftEntry, const nfd::FaceId outFaceId)
{
  if(pftEntry->getState() == pft::Entry::WAITING_IMPLICIT_ACK) {
    Face* outFace = m_faceTable->get(outFaceId);
    if(outFace->isValNetFace()) {
      outFace->sendValPacket(pftEntry->getValPacket());
      // setting ImpAck timer
      pftEntry->oneLessTry();
      if(pftEntry->getValPacket().isSet() == ValPacket::DATA_SET)
        NS_LOG_DEBUG("DATA IMP ACK retransmission!! " << pftEntry->getValPacket().getData().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos());
      else
        NS_LOG_DEBUG("INTERST IMP ACK retransmission!! " << pftEntry->getValPacket().getInterest().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos());
      if(pftEntry->getNumberOfTries() > 0) {
        pftEntry->setTimer(::nfd::scheduler::schedule(pftEntry->getDefaultImpAckTimerDuration(), [=] { impAckTimerCallback(pftEntry, outFaceId); }));
      } else {
        m_pft.removeEntry(pftEntry->getValPacket());
      }
    } else {
      NS_LOG_DEBUG("NOT ValNetFace: this should never happen!!");
    }
  }
}

std::pair<uint32_t, uint32_t>
ValForwarder::makeGeoFaceFromValHeader(const ValHeader& valH)
{
  uint32_t faceID_SA = 0;
  uint32_t faceID_DA = 0;

  auto pair_SA = m_f2a.findByGeoArea(valH.getSA());
  if(pair_SA.first) {
    faceID_SA = pair_SA.second->getFaceId();
  } else {
    auto geoface_SA = m_geofaceFactory.makeGeoface();
    m_l3P->addFace(geoface_SA);
    faceID_SA = geoface_SA->getId();
    f2a::Entry entry_SA(faceID_SA, valH.getSA());
    m_f2a.addEntry(entry_SA);
  }

  auto pair_DA = m_f2a.findByGeoArea(valH.getDA());
  if(pair_DA.first) {
    faceID_DA = pair_DA.second->getFaceId();
  } else {
    auto geoface_DA = m_geofaceFactory.makeGeoface();
    m_l3P->addFace(geoface_DA);
    faceID_DA = geoface_DA->getId();
    f2a::Entry entry_DA(faceID_DA, valH.getDA());
    m_f2a.addEntry(entry_DA);
  }

  return {faceID_SA, faceID_DA};
}
void
ValForwarder::cleanupOnFaceRemoval(const Face& face)
{
  removeFromNetworkFaceList(face);
}

Face*
ValForwarder::getNetworkFace(nfd::FaceId faceId)
{
  NS_LOG_DEBUG(__func__ << "FaceId: " << faceId);
  auto it = m_networkFaces.begin();
  while (it != m_networkFaces.end()) {
    if (*it == faceId) {
      auto it_faces = m_faceTable->begin();
      while (it_faces != m_faceTable->end()) {
        if (it_faces->getId() == faceId) {
          return &*it_faces;
        }
        it_faces++;
      }
    }
    it++;
  }
  return nullptr;
}

Face*
ValForwarder::getOtherNetworkFace(nfd::FaceId faceId)
{
  NS_LOG_DEBUG(__func__);
  auto it = m_networkFaces.begin();
  while (it != m_networkFaces.end()) {
    if (*it != faceId) {
      auto it_faces = m_faceTable->begin();
      while (it_faces != m_faceTable->end()) {
        if (it_faces->getId() == *it) {
          return &*it_faces;
        }
        it_faces++;
      }
    }
    it++;
  }
  return nullptr;
}

void
ValForwarder::addToNetworkFaceList(const Face& face)
{
  NS_LOG_DEBUG(__func__);
  m_networkFaces.push_back(face.getId());
}

void
ValForwarder::removeFromNetworkFaceList(const Face& face)
{
  NS_LOG_DEBUG(__func__);
  auto it = m_networkFaces.begin();
  while (it != m_networkFaces.end()){
    if(*it == face.getId()) {
      m_networkFaces.erase(it);
      break;
    }
    it++;
  }
}

} // namespace val            
} // namespace ndn        
} // namespace ns3