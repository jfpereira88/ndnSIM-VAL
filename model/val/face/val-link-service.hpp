/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_FACE_LINK_SERVICE_HPP
#define VAL_FACE_LINK_SERVICE_HPP

#include "../val-common.hpp"
#include "ns3/ndnSIM/NFD/daemon/face/link-service.hpp"
#include "ns3/ndnSIM/NFD/daemon/face/lp-fragmenter.hpp"
#include "ns3/ndnSIM/NFD/daemon/face/lp-reassembler.hpp"
#include "ns3/ndnSIM/NFD/daemon/face/lp-reliability.hpp"
#include "ns3/log.h"

#include <ndn-cxx/util/time.hpp>

using namespace nfd::face;


namespace ns3 {
namespace ndn {
namespace val {
namespace face {

using namespace ndn::literals::time_literals;


/** \brief counters provided by ValLinkService
 *  \note The type name 'ValLinkServiceCounters' is implementation detail.
 *        Use 'ValLinkService::Counters' in public API.
 */
class ValLinkServiceCounters : public virtual LinkService::Counters
{
public:
  /** \brief count of failed fragmentations
   */
  nfd::PacketCounter nFragmentationErrors;

  /** \brief count of outgoing LpPackets dropped due to exceeding MTU limit
   *
   *  If this counter is non-zero, the operator should enable fragmentation.
   */
  nfd::PacketCounter nOutOverMtu;

  /** \brief count of invalid LpPackets dropped before reassembly
   */
  nfd::PacketCounter nInLpInvalid;

  /** \brief count of network-layer packets currently being reassembled
   */
  nfd::SizeCounter<LpReassembler> nReassembling;

  /** \brief count of dropped partial network-layer packets due to reassembly timeout
   */
  nfd::PacketCounter nReassemblyTimeouts;

  /** \brief count of invalid reassembled network-layer packets dropped
   */
  nfd::PacketCounter nInNetInvalid;

  /** \brief count of network-layer packets that did not require retransmission of a fragment
   */
  nfd::PacketCounter nAcknowledged;

  /** \brief count of network-layer packets that had at least one fragment retransmitted, but were
   *         eventually received in full
   */
  nfd::PacketCounter nRetransmitted;

  /** \brief count of network-layer packets dropped because a fragment reached the maximum number
   *         of retransmissions
   */
  nfd::PacketCounter nRetxExhausted;

  /** \brief count of outgoing LpPackets that were marked with congestion marks
   */
  nfd::PacketCounter nCongestionMarked;

  /** \brief count of incoming ValPackets
   */
  nfd::PacketCounter nInValPkt;

  /** \brief count of outgoing ValPackets
   */
  nfd::PacketCounter nOutValPkt;
};

/** \brief ValLinkService is a LinkService that implements the NDNLPv2 protocol with ValHeader
 *  \sa https://redmine.named-data.net/projects/nfd/wiki/NDNLPv2
 */
class ValLinkService : public LinkService
                     , protected virtual ValLinkServiceCounters
{
public:
  /** \brief Options that control the behavior of ValLinkService
   */
  class Options
  {
  public:
    constexpr
    Options() noexcept
    {
    }

  public:
    /** \brief enables encoding of IncomingFaceId, and decoding of NextHopFaceId and CachePolicy
     */
    bool allowLocalFields = false;

    /** \brief enables fragmentation
     */
    bool allowFragmentation = false;

    /** \brief options for fragmentation
     */
    LpFragmenter::Options fragmenterOptions;

    /** \brief enables reassembly
     */
    bool allowReassembly = false;

    /** \brief options for reassembly
     */
    LpReassembler::Options reassemblerOptions;

    /** \brief enables send queue congestion detection and marking
     */
    bool allowCongestionMarking = false;

    /** \brief starting value for congestion marking interval
     *
     *  The default value (100 ms) is taken from RFC 8289 (CoDel).
     */
    time::nanoseconds baseCongestionMarkingInterval = 100_ms;

    /** \brief default congestion threshold in bytes
     *
     *  The default value (64 KiB) works well for a queue capacity of 200 KiB.
     */
    size_t defaultCongestionThreshold = 65536;

    /** \brief enables self-learning forwarding support
     */
    bool allowSelfLearning = true;
  };

  /** \brief counters provided by ValLinkService
   */
  using Counters = ValLinkServiceCounters;

  explicit
  ValLinkService(const Options& options = {});

  /** \brief get Options used by ValLinkService
   */
  const Options&
  getOptions() const;

  /** \brief sets Options used by ValLinkService
   */
  void
  setOptions(const Options& options);

  const Counters&
  getCounters() const override;

private: // send path 
  /** \brief send an LpPacket fragment
   *  send to transport using LinkService::sendPacket()
   *  \param pkt LpPacket to send
   */
  void
  sendLpPacket(lp::Packet&& pkt);

  /** \brief encode link protocol fields from tags onto an outgoing LpPacket
   *  \param netPkt network-layer packet to extract tags from
   *  \param lpPacket LpPacket to add link protocol fields to
   */
  void
  encodeLpFields(const ndn::PacketBase& netPkt, lp::Packet& lpPacket);

  /** \brief send a complete Val layer packet
   *  \param pkt LpPacket containing a complete Val  layer packet
   */
  void
  sendValPacket(lp::Packet&& pkt);

  /** \brief assign a sequence number to an LpPacket
   */
  void
  assignSequence(lp::Packet& pkt);

  /** \brief assign consecutive sequence numbers to LpPackets
   */
  void
  assignSequences(std::vector<lp::Packet>& pkts);

  /** \brief if the send queue is found to be congested, add a congestion mark to the packet
   *         according to CoDel
   *  \sa https://tools.ietf.org/html/rfc8289
   */
  void
  checkCongestionLevel(lp::Packet& pkt);
  
  /** \brief method that is called by LinkService to send a Val packet
   *  \param valPacket a val layer packet
   */
  void
  doSendValPacket(const ::ns3::ndn::val::ValPacket& valPacket) override;

private: // receive path
  /** \brief receive Packet from Transport
   */
  void
  doReceivePacket(Transport::Packet&& packet) override;
  
  /**
   * \brief This function tags the Interest packet with the fields of th NDNLPv2 Header
   */
  std::shared_ptr<::ndn::Interest>
  decodeInterest(const Block& netPkt, const lp::Packet& firstPkt);

  /**
   * \brief This function tags the Data packet with the fields of th NDNLPv2 Header
   */
  std::shared_ptr<::ndn::Data>
  decodeData(const Block& netPkt, const lp::Packet& firstPkt);

private: // pure virtual methods from LinkService that are not used but need to be define
  
  /**
   * \brief overrides base class method and sends - making it do nothing
   */
  void
  doSendInterest(const Interest& interest) override;

  /**
   * \brief overrides base class method and sends - making it do nothing
   */
  void
  doSendData(const Data& data) override;

  /**
   * \brief overrides base class method and sends - making it do nothing
   */
  void
  doSendNack(const lp::Nack& nack) override;

  Options m_options;
  LpFragmenter m_fragmenter;
  LpReassembler m_reassembler;
  lp::Sequence m_lastSeqNo;

  /// CongestionMark TLV-TYPE (3 octets) + CongestionMark TLV-LENGTH (1 octet) + sizeof(uint64_t)
  static constexpr size_t CONGESTION_MARK_SIZE = 3 + 1 + sizeof(uint64_t);

  /// Time to mark next packet due to send queue congestion
  time::steady_clock::TimePoint m_nextMarkTime;
  /// Time last packet was marked
  time::steady_clock::TimePoint m_lastMarkTime;
  /// number of marked packets in the current incident of congestion
  size_t m_nMarkedSinceInMarkingState;

};

inline const ValLinkService::Options&
ValLinkService::getOptions() const
{
  return m_options;
}

inline const ValLinkService::Counters&
ValLinkService::getCounters() const
{
  return *this;
}

} // namespace face    
} // namespace val
} // namespace ndn
} // namespace ns3

#endif //VAL_FACE_LINK_SERVICE_HPP