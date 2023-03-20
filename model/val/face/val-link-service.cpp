/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-link-service.hpp"

#include "../val-packet.hpp"

#include "ns3/log.h"

#include <ndn-cxx/lp/tags.hpp>

#include <boost/stacktrace.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.val.face.ValLinkService");

namespace ns3 {
namespace ndn {
namespace val {
namespace face {


constexpr uint32_t DEFAULT_CONGESTION_THRESHOLD_DIVISOR = 2;

ValLinkService::ValLinkService(const ValLinkService::Options& options)
  : m_options(options)
  , m_fragmenter(m_options.fragmenterOptions, this)
  , m_reassembler(m_options.reassemblerOptions, this)
  , m_lastSeqNo(-2)
  , m_nextMarkTime(time::steady_clock::TimePoint::max())
  , m_lastMarkTime(time::steady_clock::TimePoint::min())
  , m_nMarkedSinceInMarkingState(0)
{
  m_reassembler.beforeTimeout.connect([this] (auto...) { ++this->nReassemblyTimeouts; });
  nReassembling.observe(&m_reassembler);
}

void
ValLinkService::setOptions(const ValLinkService::Options& options)
{
  m_options = options;
  m_fragmenter.setOptions(m_options.fragmenterOptions);
  m_reassembler.setOptions(m_options.reassemblerOptions);
}

// send packet exit point
void
ValLinkService::sendLpPacket(lp::Packet&& pkt)
{
  const ssize_t mtu = this->getTransport()->getMtu();

  if (m_options.allowCongestionMarking) {
    checkCongestionLevel(pkt);
  }

  Transport::Packet tp(pkt.wireEncode());
  if (mtu != MTU_UNLIMITED && tp.packet.size() > static_cast<size_t>(mtu)) {
    ++this->nOutOverMtu;
    NS_LOG_WARN("attempted to send packet over MTU limit");
    return;
  }
  NS_LOG_DEBUG("send packet to transport");
  this->sendPacket(std::move(tp));
}

void
ValLinkService::encodeLpFields(const ndn::PacketBase& netPkt, lp::Packet& lpPacket)
{
  if (m_options.allowLocalFields) {
    shared_ptr<lp::IncomingFaceIdTag> incomingFaceIdTag = netPkt.getTag<lp::IncomingFaceIdTag>();
    if (incomingFaceIdTag != nullptr) {
      lpPacket.add<lp::IncomingFaceIdField>(*incomingFaceIdTag);
    }
  }

  shared_ptr<lp::CongestionMarkTag> congestionMarkTag = netPkt.getTag<lp::CongestionMarkTag>();
  if (congestionMarkTag != nullptr) {
    lpPacket.add<lp::CongestionMarkField>(*congestionMarkTag);
  }

  if (m_options.allowSelfLearning) {
    shared_ptr<lp::NonDiscoveryTag> nonDiscoveryTag = netPkt.getTag<lp::NonDiscoveryTag>();
    if (nonDiscoveryTag != nullptr) {
      lpPacket.add<lp::NonDiscoveryField>(*nonDiscoveryTag);
    }

    shared_ptr<lp::PrefixAnnouncementTag> prefixAnnouncementTag = netPkt.getTag<lp::PrefixAnnouncementTag>();
    if (prefixAnnouncementTag != nullptr) {
      lpPacket.add<lp::PrefixAnnouncementField>(*prefixAnnouncementTag);
    }
  }

  shared_ptr<lp::HopCountTag> hopCountTag = netPkt.getTag<lp::HopCountTag>();
  if (hopCountTag != nullptr) {
    lpPacket.add<lp::HopCountTagField>(*hopCountTag);
  }
  else {
    lpPacket.add<lp::HopCountTagField>(0);
  }
}

void
ValLinkService::sendValPacket(lp::Packet&& pkt)
{
  std::vector<lp::Packet> frags;
  ssize_t mtu = this->getTransport()->getMtu();

  if (m_options.allowCongestionMarking && mtu != MTU_UNLIMITED) {
    mtu -= CONGESTION_MARK_SIZE;
  }

  BOOST_ASSERT(mtu == MTU_UNLIMITED || mtu > 0);

  if (m_options.allowFragmentation && mtu != MTU_UNLIMITED) {
    bool isOk = false;
    std::tie(isOk, frags) = m_fragmenter.fragmentPacket(pkt, mtu);
    if (!isOk) {
      // fragmentation failed (warning is logged by LpFragmenter)
      ++this->nFragmentationErrors;
      return;
    }
  }
  else {
      frags.push_back(std::move(pkt));
  }

  if (frags.size() == 1) {
    // even if indexed fragmentation is enabled, the fragmenter should not
    // fragment the packet if it can fit in MTU
    BOOST_ASSERT(!frags.front().has<lp::FragIndexField>());
    BOOST_ASSERT(!frags.front().has<lp::FragCountField>());
  }

  // Only assign sequences to fragments if packet contains more than 1 fragment
  if (frags.size() > 1) {
    // Assign sequences to all fragments
    this->assignSequences(frags);
  }

  //if (m_options.reliabilityOptions.isEnabled && frags.front().has<lp::FragmentField>()) {
  //  m_reliability.handleOutgoing(frags, std::move(pkt), isInterest);
  //}

  for (lp::Packet& frag : frags) {
    this->sendLpPacket(std::move(frag));
  }
}

void
ValLinkService::assignSequence(lp::Packet& pkt)
{
  pkt.set<lp::SequenceField>(++m_lastSeqNo);
}

void
ValLinkService::assignSequences(std::vector<lp::Packet>& pkts)
{
  std::for_each(pkts.begin(), pkts.end(), [this] (auto& pkt) { this->assignSequence(pkt); });
}

void
ValLinkService::checkCongestionLevel(lp::Packet& pkt)
{
  ssize_t sendQueueLength = getTransport()->getSendQueueLength();
  // This operation requires that the transport supports retrieving current send queue length
  if (sendQueueLength < 0) {
    return;
  }

  // To avoid overflowing the queue, set the congestion threshold to at least half of the send
  // queue capacity.
  size_t congestionThreshold = m_options.defaultCongestionThreshold;
  if (getTransport()->getSendQueueCapacity() >= 0) {
    congestionThreshold = std::min(congestionThreshold,
                                   static_cast<size_t>(getTransport()->getSendQueueCapacity()) /
                                                       DEFAULT_CONGESTION_THRESHOLD_DIVISOR);
  }

  if (sendQueueLength > 0) {
    NS_LOG_DEBUG("txqlen=" << sendQueueLength << " threshold=" << congestionThreshold <<
                       " capacity=" << getTransport()->getSendQueueCapacity());
  }

  if (static_cast<size_t>(sendQueueLength) > congestionThreshold) { // Send queue is congested
    const auto now = time::steady_clock::now();
    if (now >= m_nextMarkTime || now >= m_lastMarkTime + m_options.baseCongestionMarkingInterval) {
      // Mark at most one initial packet per baseCongestionMarkingInterval
      if (m_nMarkedSinceInMarkingState == 0) {
        m_nextMarkTime = now;
      }

      // Time to mark packet
      pkt.set<lp::CongestionMarkField>(1);
      ++nCongestionMarked;
      NS_LOG_DEBUG("LpPacket was marked as congested");

      ++m_nMarkedSinceInMarkingState;
      // Decrease the marking interval by the inverse of the square root of the number of packets
      // marked in this incident of congestion
      m_nextMarkTime += time::nanoseconds(static_cast<time::nanoseconds::rep>(
                                            m_options.baseCongestionMarkingInterval.count() /
                                            std::sqrt(m_nMarkedSinceInMarkingState)));
      m_lastMarkTime = now;
    }
  }
  else if (m_nextMarkTime != time::steady_clock::TimePoint::max()) {
    // Congestion incident has ended, so reset
    NS_LOG_DEBUG("Send queue length dropped below congestion threshold");
    m_nextMarkTime = time::steady_clock::TimePoint::max();
    m_nMarkedSinceInMarkingState = 0;
  }
}

// receive packet entry point
void
ValLinkService::doReceivePacket(Transport::Packet&& packet) 
{
  try {
    lp::Packet pkt(packet.packet);
    if (!pkt.has<lp::FragmentField>()) {
      NS_LOG_ERROR("received IDLE packet: DROP");
      return;
    }

    if ((pkt.has<lp::FragIndexField>() || pkt.has<lp::FragCountField>()) &&
        !m_options.allowReassembly) {
      NS_LOG_WARN("received fragment, but reassembly disabled: DROP");
      return;
    }

    bool isReassembled = false;
    Block netPkt;
    lp::Packet firstPkt;
    std::tie(isReassembled, netPkt, firstPkt) = m_reassembler.receiveFragment(packet.remoteEndpoint,
                                                                              pkt);
    if (isReassembled) {
      // at this point we have the NDNPacket in wire format (netPkt)
      // and the NDNLPv2 Headers (firstPkt)
      // we need to extract ValHeader from firstPkt
      ndn::Block wire = firstPkt.wireEncode();
      for(ndn::Block block : wire.elements()) {
        if(block.type() == ::ndn::lp::tlv::ValHeader) {
          block.parse(); // to get all the tlv subelements
          ValHeader valH(block); // we now have the ValHeader object
          NS_LOG_DEBUG("ValHeader SA: " << valH.getSA());
          NS_LOG_DEBUG("ValHeader DA: " << valH.getDA());
          NS_LOG_DEBUG("ValHeader phPOS: " << valH.getPhPos());
          NS_LOG_DEBUG("ValHeader RN: " << valH.getRN());
          NS_LOG_DEBUG("ValHeader hopC: " << std::to_string(valH.getHopC()));
          // now lets create the ValPacket
          // the ValPacket is just an object that agregates the
          // ValHeader and the NDNPacket (data or interest)
          // this NDNPacket first needs to be tagged with the information 
          // from the Headers of the NDNLPv2 protocol
          // this tagged information is not used, but it can be usefull in future work
          ValPacket valPkt(valH);
          switch (netPkt.type())
          {
          case ::ndn::tlv::Interest:
          {
            valPkt.setInterest(decodeInterest(netPkt, firstPkt));
            break;
          }
          case ::ndn::tlv::Data:
          {
            valPkt.setData(decodeData(netPkt, firstPkt));
            break;
          }  
          default:
            NS_LOG_ERROR("this should never happen!");
            break;
          }
          ++this->nInValPkt;
          // send to val-forwarder here
          NS_LOG_DEBUG("sending val packet to valforwarder");
          this->receiveValPacket(std::move(valPkt));
          break;
        }
      }
    }
  }
  catch (const tlv::Error& e) {
    ++this->nInLpInvalid;
    NS_LOG_WARN("packet parse error (" << e.what() << "): DROP");
  }
}

std::shared_ptr<::ndn::Interest>
ValLinkService::decodeInterest(const Block& netPkt, const lp::Packet& firstPkt)
{
  BOOST_ASSERT(netPkt.type() == tlv::Interest);
  BOOST_ASSERT(!firstPkt.has<lp::NackField>());

  // forwarding expects Interest to be created with make_shared
  auto interest = make_shared<Interest>(netPkt);
  NS_LOG_DEBUG("Interest: " << interest->getName().toUri());

  // Increment HopCount
  if (firstPkt.has<lp::HopCountTagField>()) {
    interest->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    if (m_options.allowLocalFields) {
      interest->setTag(make_shared<lp::NextHopFaceIdTag>(firstPkt.get<lp::NextHopFaceIdField>()));
    }
    else {
      NS_LOG_WARN("received NextHopFaceId, but local fields disabled: DROP");
      return nullptr;
    }
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    ++this->nInNetInvalid;
    NS_LOG_WARN("received CachePolicy with Interest: DROP");
    return nullptr;
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NS_LOG_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    interest->setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    if (m_options.allowSelfLearning) {
      interest->setTag(make_shared<lp::NonDiscoveryTag>(firstPkt.get<lp::NonDiscoveryField>()));
    }
    else {
      NS_LOG_WARN("received NonDiscovery, but self-learning disabled: IGNORE");
    }
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    ++this->nInNetInvalid;
    NS_LOG_WARN("received PrefixAnnouncement with Interest: DROP");
    return nullptr;
  }

  return interest;
}

std::shared_ptr<::ndn::Data>
ValLinkService::decodeData(const Block& netPkt, const lp::Packet& firstPkt)
{
  BOOST_ASSERT(netPkt.type() == tlv::Data);

  // forwarding expects Data to be created with make_shared
  auto data = make_shared<Data>(netPkt);
  NS_LOG_DEBUG("Data: " << data->getName().toUri());

  if (firstPkt.has<lp::HopCountTagField>()) {
    data->setTag(make_shared<lp::HopCountTag>(firstPkt.get<lp::HopCountTagField>() + 1));
  }

  if (firstPkt.has<lp::NackField>()) {
    ++this->nInNetInvalid;
    NS_LOG_WARN("received Nack with Data: DROP");
    return nullptr;
  }

  if (firstPkt.has<lp::NextHopFaceIdField>()) {
    ++this->nInNetInvalid;
    NS_LOG_WARN("received NextHopFaceId with Data: DROP");
    return nullptr;
  }

  if (firstPkt.has<lp::CachePolicyField>()) {
    // CachePolicy is unprivileged and does not require allowLocalFields option.
    // In case of an invalid CachePolicyType, get<lp::CachePolicyField> will throw,
    // so it's unnecessary to check here.
    data->setTag(make_shared<lp::CachePolicyTag>(firstPkt.get<lp::CachePolicyField>()));
  }

  if (firstPkt.has<lp::IncomingFaceIdField>()) {
    NS_LOG_WARN("received IncomingFaceId: IGNORE");
  }

  if (firstPkt.has<lp::CongestionMarkField>()) {
    data->setTag(make_shared<lp::CongestionMarkTag>(firstPkt.get<lp::CongestionMarkField>()));
  }

  if (firstPkt.has<lp::NonDiscoveryField>()) {
    ++this->nInNetInvalid;
    NS_LOG_WARN("received NonDiscovery with Data: DROP");
    return nullptr;
  }

  if (firstPkt.has<lp::PrefixAnnouncementField>()) {
    if (m_options.allowSelfLearning) {
      data->setTag(make_shared<lp::PrefixAnnouncementTag>(firstPkt.get<lp::PrefixAnnouncementField>()));
    }
    else {
      NS_LOG_WARN("received PrefixAnnouncement, but self-learning disabled: IGNORE");
    }
  }

  return data;
}

// send packet entry point
void
ValLinkService::doSendValPacket(const ::ns3::ndn::val::ValPacket& valPacket)
{
  NS_LOG_DEBUG(__func__);
  // Here we receive a ValPacket that contains the ValHeader
  // and a tagged NDNPacket we need to pass the tagged information
  // to the NDNPLv2 packet (lpPacket)
  // as also the ValHeader information
  
  switch (valPacket.isSet()) {
      case ValPacket::INTEREST_SET:
      { 
        Interest interest(valPacket.getInterest());
        lp::Packet lpPacket(interest.wireEncode());
        encodeLpFields(interest, lpPacket);
        NS_LOG_DEBUG("before adding ValHeader to NDNLPV2");
        lpPacket.add<lp::ValHeaderField>(valPacket.getValHeader());
        NS_LOG_DEBUG("after adding ValHeader to NDNLPV2");
        this->sendValPacket(std::move(lpPacket));
        break;
      }
      case ValPacket::DATA_SET:
      {
        Data data(valPacket.getData());
        lp::Packet lpPacket(data.wireEncode());
        encodeLpFields(data, lpPacket);
        lpPacket.add<lp::ValHeaderField>(valPacket.getValHeader());
        this->sendValPacket(std::move(lpPacket));
        break;
      }
      default:
        NS_LOG_DEBUG("unrecognized network-layer packet TLV-TYPE " << valPacket.isSet() << ": DROP");
        return;
    }
}


// override pure virtual methods
void
ValLinkService::doSendInterest(const Interest& interest)
{
    NS_LOG_ERROR("ValLinkService::doSendInterest -> This should never happen!!!");
    std::cout << boost::stacktrace::stacktrace();
    BOOST_ASSERT(false);
}

void
ValLinkService::doSendData(const Data& data)
{
    NS_LOG_ERROR("ValLinkService::doSendData -> This should never happen!!!");
    BOOST_ASSERT(false);
}

void
ValLinkService::doSendNack(const lp::Nack& nack)
{
    NS_LOG_ERROR("ValLinkService::doSendNack -> This should never happen!!!");
    BOOST_ASSERT(false);
}


} // namespace face
} // namespace val
}
}
