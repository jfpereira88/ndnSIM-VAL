/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-forwarder.hpp"
#include "val-distances-strategy.hpp"
#include "ns3/ndnSIM/model/ndn-l3-protocol.hpp"
#include "ns3/ndnSIM/model/ndn-net-device-transport.hpp"
#include "ns3/log.h"
#include<iostream>
#include<fstream>

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
    , m_inValPkts(0)
    , m_outValPkts(0)
    , m_inDataPkts(0)
    , m_inIntPkts(0)
    , m_outDataPkts(0)
    , m_outIntPkts(0)
    , m_rtxDataPkts(0)
    , m_rtxIntPkts(0)
    , m_HLT_to_RTX(0)
    , m_commValFace(m_geofaceFactory.makeCommValFace())
{
    NS_LOG_DEBUG("Creating VAL"); 
    m_faceTable = &(m_l3P->getForwarder()->getFaceTable());
    m_faceTable->afterValFaceAdd.connect([this] (Face& face) {
      NS_LOG_DEBUG("Adding ValNetFace? "<< std::boolalpha << face.isValNetFace());
      addToNetworkFaceList(face);
      m_strategy->setMobilityModel(dynamic_cast<ns3::ndn::NetDeviceTransport*>(face.getTransport())->GetNetDevice()->GetNode()->GetObject<MobilityModel>());
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
    m_faceTable->addReserved(m_commValFace, face::GeofaceFactory::FACEID_COMMVALFACE);
    auto floodFace = m_geofaceFactory.makeGeoface();
    m_l3P->addFace(floodFace);  // adding face to face table
    nfd::fib::Entry* entry = m_l3P->getForwarder()->getFib().insert("/").first;
    entry->addOrUpdateNextHop(*floodFace, 0, 1);
    f2a::Entry floodFaceEntry(floodFace->getId(), "0");  // this can only be made after adding face to face table
    m_f2a.addEntry(floodFaceEntry);
}

ValForwarder::~ValForwarder()
{
  uint32_t nodeId = dynamic_cast<ns3::ndn::NetDeviceTransport*>(m_faceTable->get(m_networkFaces.front())->getTransport())->GetNetDevice()->GetNode()->GetId();
  NS_LOG_INFO("Node Type Packets");
  NS_LOG_INFO("" << nodeId << " InValPkts " << m_inValPkts);
  NS_LOG_INFO("" << nodeId << " InIntPkts " << m_inIntPkts);
  NS_LOG_INFO("" << nodeId << " InDataPkts " << m_inDataPkts);
  NS_LOG_INFO("" << nodeId << " OutValPkts " << m_outValPkts);
  NS_LOG_INFO("" << nodeId << " OutIntPkts " << m_outIntPkts);
  NS_LOG_INFO("" << nodeId << " OutDataPkts " << m_outDataPkts);
  NS_LOG_INFO("" << nodeId << " RtxIntPkts " << m_rtxIntPkts);
  NS_LOG_INFO("" << nodeId << " RtxDataPkts " << m_rtxDataPkts);

  std::ofstream file;
  std::string name = "val_pkts_" + std::to_string(nodeId) + ".txt";
  file.open(name);

  //file << "Node Type Packets"
  file << nodeId << " InValPkts " << m_inValPkts
      << "\n" << nodeId << " InIntPkts " << m_inIntPkts
      << "\n" << nodeId << " InDataPkts " << m_inDataPkts
      << "\n" << nodeId << " OutValPkts " << m_outValPkts
      << "\n" << nodeId << " OutIntPkts " << m_outIntPkts
      << "\n" << nodeId << " OutDataPkts " << m_outDataPkts
      << "\n" << nodeId << " RtxIntPkts " << m_rtxIntPkts
      << "\n" << nodeId << " RtxDataPkts " << m_rtxDataPkts
      << "\n" << nodeId << " HLTtoRTX " << m_HLT_to_RTX
      << std::endl;

  file.close();

  delete m_strategy;
}

void
ValForwarder::onReceivedValPacket(const Face& face, const ValPacket& valP)
{
  NS_LOG_DEBUG(__func__);
  m_inValPkts++;
  if(valP.isSet () == ValPacket::DATA_SET)
    m_inDataPkts++;
  else
    m_inIntPkts++;
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
  NS_LOG_DEBUG(__func__);
  // the first thing to do is to check it agains PFT and see if it is a
  // transmission of a node in a better position or an Implicit ACK
  // the PFT - Pending Forwarding Table entries identify both scenarios
  auto pair = m_pft.findMatch(valP);
  if(pair.first) // match found
  {
    int prev_stage = pair.second->getState();
    int state = pair.second->changeState(valP.getValHeader().getHopC(), false);
    NS_LOG_DEBUG ("Location net: " << valP.getValHeader().getPhPos() << " Location pft: "
    << pair.second->getValPacket().getValHeader().getPhPos());
    NS_LOG_DEBUG ("hopCnet: " <<
                      std::to_string(valP.getValHeader().getHopC()) << " hopCpft: " <<
                      std::to_string(pair.second->getHopC()));
    NS_LOG_DEBUG ("stage changed from: " << std::to_string(prev_stage) << " to: " << std::to_string(state));
    switch (state)
    {
    case pft::Entry::HLT:
      if (prev_stage == pft::Entry::FWD)
      {
        ::nfd::scheduler::cancel(pair.second->getTimerId()); // cancel FWD timer
        // set self-clean timer
        pair.second->setTimer(::nfd::scheduler::schedule(pair.second->getTimerDelay(), [=] { selfCleanTimerCallback(pair.second); }));
        NS_LOG_DEBUG ("HLT Self Cleaning timer set");
      }
      break;
    
    case pft::Entry::RTX:
      if (prev_stage == pft::Entry::HLT)
      {
        ::nfd::scheduler::cancel(pair.second->getTimerId());
        pair.second->setTimer(::nfd::scheduler::schedule(pair.second->getTimerDelay(), [=, &face] { retransmissionTimerCallback(pair.second, face.getId()); }));
        NS_LOG_DEBUG ("RTX timer set after HLT");
        m_HLT_to_RTX++;
      }
      break;

    case pft::Entry::IMP_ACK:
        if(prev_stage != pft::Entry::IMP_ACK)
        {
          ::nfd::scheduler::cancel(pair.second->getTimerId());
          pair.second->setTimer(::nfd::scheduler::schedule(pair.second->getTimerDelay(), [=] { selfCleanTimerCallback(pair.second); }));
          NS_LOG_DEBUG ("IMP_ACK Self Cleaning timer set");
        }
      break;
    
     case pft::Entry::ERROR:
      NS_LOG_DEBUG ("ERROR: State Machine not working");
      break;

     default:
      break;
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
  NS_LOG_DEBUG("Node at:" << dynamic_cast<ValDistancesStrategy*>(m_strategy)->positionCheck());
  NS_LOG_DEBUG("data: " << data.getFullName().toUri());

  if(inGeoface->getLocalUri().toString() == face::GeofaceFactory::STR_commValFace) {
    NS_LOG_DEBUG("Received data from COMMVALFACE");
  }
  if(data.getCongestionMark() == 501) {
    NS_LOG_DEBUG("cleaning due to unsolicited data");
    auto to_remove = m_dfnt.findMatch(data.getSignature());
    data.removeTag<lp::CongestionMarkTag>();
    if(to_remove.first) {
        if( to_remove.second->getHopC () > 0) {
          //m_strategy->unsolisitedData (to_remove.second, data);
          //NS_LOG_DEBUG ("sending unsolicited data");
        }
      m_dfnt.removeEntry(*to_remove.second);
    }
    return;
  }
  // get ifnt entries
  ifnt::ListMatchResult ifntEntriesList = m_ifnt.findMatchByNonceList(nonceList);
  
  auto pair = m_dfnt.findMatch(data.getSignature());
  if(pair.first && !isProducer) { // relay node
    NS_LOG_DEBUG("Data from network");
    // check agains PFT for cancelling retransmission of pending Interests
    // Data can serve as an Implicit ACK or even as Interest Forwarding cancelation
    pft::ListMatchResult pendingInterests = m_pft.findMatchByNonceList(nonceList);
    for(auto it = pendingInterests.begin(); it != pendingInterests.end(); it++) {
      ::nfd::scheduler::cancel((*it)->getTimerId());
      m_pft.removeEntry((*it)->getValPacket());
    }
    m_pft.removeEntriesByMatchList(pendingInterests);
    m_dfnt.removeEntry(*pair.second); // this may cause trouble
    if(data.getCongestionMark() != 500) { // if 500 just clean entries and discards packet
      m_strategy->afterDfntHit(inGeoface->getId(), pair.second, &ifntEntriesList, data);
    } else {
      NS_LOG_DEBUG("Control comamnd: cleaning");
      data.removeTag<lp::CongestionMarkTag>();
      m_strategy->sendZeroData(pair.second, data);  // ACK
      NS_LOG_DEBUG("Zero Data Sent");
      //m_dfnt.removeEntry(*pair.second); // this may cause trouble
    }
  } else {  // generated locally
    if(ifntEntriesList.size () > 0 && *ifntEntriesList.begin() != nullptr) {
      NS_LOG_DEBUG("Data generated locally");
      m_strategy->afterDfntMiss(inGeoface->getId(), data, &ifntEntriesList, isProducer);
    } else {
      NS_LOG_ERROR("Nothing to do");
    }
    if(pair.first){
      m_dfnt.removeEntry(*pair.second); // this may cause trouble
    }
  }
  m_ifnt.removeEntriesByNonceList(nonceList);
}

void
ValForwarder::registerOutgoingValPacket(const nfd::FaceId outFaceId, ValPacket& valPkt, time::nanoseconds duration)
{
  // creates pft entry and schedules the forwarding events
  if(valPkt.getValHeader().getHopC() == 0)
    return;
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
  if(pftEntry->getState() == pft::Entry::FWD) {
    NS_LOG_DEBUG("Fire forwarding timer callback");
    Face* outFace = m_faceTable->get(outFaceId);
    if(outFace->isValNetFace()) {
      outFace->sendValPacket(pftEntry->getValPacket());
      m_outValPkts++;
      if(pftEntry->getValPacket().isSet() == ValPacket::DATA_SET) {
        m_outDataPkts++;
        NS_LOG_DEBUG("DATA Forwarding timer " << pftEntry->getValPacket().getData().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos());
       } else {
        m_outIntPkts++;
        NS_LOG_DEBUG("INTERST Forwarding timer " << pftEntry->getValPacket().getInterest().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos());
       }
      // setting ImpAck timer
      int prv_state = pftEntry->getState();
      int state = pftEntry->changeState(9, true);  // changing state
      NS_LOG_DEBUG ("stage changed from: " << std::to_string(prv_state) << " to: " << std::to_string(state));
      pftEntry->setTimer(::nfd::scheduler::schedule(pftEntry->getTimerDelay(), [=] { retransmissionTimerCallback(pftEntry, outFaceId); }));
      // Data Last hop does not receive ImpACK
     /*if(pftEntry->getValPacket().isSet() == ValPacket::DATA_SET && pftEntry->getValPacket().getValHeader().getHopC() <= 1) {
        ::nfd::scheduler::cancel(pftEntry->getTimerId());
        m_pft.removeEntry(pftEntry->getValPacket());
        NS_LOG_DEBUG("Removing entry");
      }*/
    } else {
      NS_LOG_DEBUG("NOT ValNetFace: this should never happen");
    }
  }
}

void
ValForwarder::retransmissionTimerCallback(const std::shared_ptr<pft::Entry>& pftEntry, const nfd::FaceId outFaceId)
{
  if(pftEntry->getState() == pft::Entry::RTX && pftEntry->getNumberOfTries () > 0) {
    Face* outFace = m_faceTable->get(outFaceId);
    if(outFace->isValNetFace()) {
      // update position
      std::string current_pos = dynamic_cast<ValDistancesStrategy*>(m_strategy)->positionCheck();
      pftEntry->updatePhPos(current_pos);
      outFace->sendValPacket(pftEntry->getValPacket());
      m_outValPkts++;

      if(pftEntry->getValPacket().isSet() == ValPacket::DATA_SET){
        NS_LOG_DEBUG("DATA pkt retransmission " << pftEntry->getValPacket().getData().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos() << " number of tries: " << std::to_string (pftEntry->getNumberOfTries()));
        m_outDataPkts++;
        m_rtxDataPkts++;
      } else {
        NS_LOG_DEBUG("INTERST pkt retransmission " << pftEntry->getValPacket().getInterest().getName().toUri()
          << " hopC " << std::to_string(pftEntry->getValPacket().getValHeader().getHopC())
          << " my location: " << pftEntry->getValPacket().getValHeader().getPhPos() << " number of tries: " << std::to_string (pftEntry->getNumberOfTries()));
        m_outIntPkts++;
        m_rtxIntPkts++;
      }

      // setting ImpAck timer
      pftEntry->oneLessTry();
      if(pftEntry->getNumberOfTries() > 0) {
        pftEntry->setTimer(::nfd::scheduler::schedule(pftEntry->getTimerDelay(), [=] { retransmissionTimerCallback(pftEntry, outFaceId); }));
      } else {
        m_pft.removeEntry(pftEntry->getValPacket());
      }
    } else {
      NS_LOG_DEBUG("NOT ValNetFace: this should never happen");
    }
  } else {
      m_pft.removeEntry(pftEntry->getValPacket());
  }
}

void
ValForwarder::selfCleanTimerCallback(const std::shared_ptr<pft::Entry>& pftEntry)
{
  NS_LOG_DEBUG(__func__);
  NS_LOG_DEBUG ("Cleaning PFT Entry");
  m_pft.removeEntry(pftEntry->getValPacket());
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
