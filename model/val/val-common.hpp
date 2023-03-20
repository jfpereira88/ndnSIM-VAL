/**
 * \mainpage
 * Vehicle Adaptation Layer (VAL) is an adaptation of ndnSim that allows for geographical-based forwarding in VANETs.
 * 
 * VAL endows ndnSim with a second forwarding plane capable of making forwarding decisions based on the destination area of a packet and the position of the previous-hop.
 * 
 * VAL serves the NFD forwarding plane, does not replace it. It respects layer independence and does not alter the NDN primitives or core structures.
 * 
 * VAL implements a forwarding strategy based on distances, other forwarding strategies can be developed using \c ValStrategy as base class.
 *
 * The design provides:
 * - (1) a mechanism to bind NDN data names to the producers’ and consumers' geographic areas; (geoFaces)
 * - (2) an algorithm to guide Interests towards data producers using the distances between nodes and distances to the producers' areas;  ( \c ValDistancesStrategy)
 * - (3) an algorithm to guide Data packets towards data consumers' using the distances between nodes; ( \c ValDistancesStrategy)
 * - (4) a discovery mechanism to find producers' areas. (discovery phase: using a special geoFace with no associated geo-area that is the default route in every FIB, the floodFace)
 * - (5) a mechanism to mitigate Broadcast Storms. (Forwarding cancellation based on timers and retransmission of packets from node in better position)
 * - (6) a mechanism that mitigates intermittent communications characteristic of VANETs (implicit acknowlegment)
 * 
 * This work is based on the LAL presented in NAVIGO
 * \sa https://ieeexplore.ieee.org/abstract/document/7158165
 * 
 * VAL is still in an embryonic stage of development.
 * 
 * Masters Dissertion:
 * 
 * jfp 2019
 * 
 * mieti - uminho
 */

#ifndef VAL_COMMON_HPP
#define VAL_COMMON_HPP

#include <ndn-cxx/util/time.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/signature.hpp>
#include <ndn-cxx/util/signal.hpp>
#include <ndn-cxx/data.hpp>
#include <ndn-cxx/interest.hpp>

#include <memory>

#include "ns3/ndnSIM/NFD/daemon/face/face.hpp"


namespace ns3 {
namespace ndn {
namespace val {

namespace ndn {
    using namespace ::ndn;
} // namespace ndn

namespace time {
    using namespace ::ndn::time;
} // namespace time

namespace lp {
    using namespace ::ndn::lp;
} // namespace lp

namespace tlv {
    using namespace ::ndn::tlv;
} // namespace tlv

namespace signal {
    using namespace ::ndn::util::signal;
} 

namespace nfd {
    using namespace ::nfd;
} // namespace nfd

using std::shared_ptr;
using std::make_shared;

using ::ndn::Interest;
using ::ndn::Data;
using ::ndn::Signature;
using ::ndn::Block;
using ::ndn::FaceUri;
using ::ndn::make_unique;

using ::nfd::face::Face;

    
} // namespace val    
} // namespace ndn
} // namespace ns3


#endif // VAL_COMMON_HPP