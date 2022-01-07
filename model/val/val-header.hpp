/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_HEADER_HPP
#define VAL_HEADER_HPP

#include <ndn-cxx/encoding/block.hpp>
#include "ndn-cxx/encoding/encoding-buffer.hpp"
#include "ndn-cxx/lp/tlv.hpp"

namespace ns3 {
namespace ndn {
namespace val {

using ::ndn::Block;

/**
 * \brief ValHeader - aggregates the geographical information that travels between nodes.
 * 
 * That information can be in TLV format or as member variables within the object.
 */
class ValHeader
{
public:
  static constexpr uint8_t MAXHOPS = 9;

  class Error : public ::ndn::tlv::Error
  {
  public:
    using ::ndn::tlv::Error::Error;
  };
  /**
   * \brief Dummy constructor - just for tests
   */
  ValHeader();
  
  /**
   * \brief constructs object from variables 
   */
  ValHeader(std::string SA, std::string DA, std::string phPos,
                        std::string RN, uint8_t hopC);

  /**
   * \brief constructs object from TLV element
   */
  explicit
  ValHeader(const Block& block);

  /**
   * \brief prepend ValHeader to encoder
   */
  template<::ndn::encoding::Tag TAG>
  size_t
  wireEncode(::ndn::EncodingImpl<TAG>& encoder) const;

  /**
   * \brief encode ValHeader into wire format
   */
  const Block&
  wireEncode() const;

  /**
   * \brief get ValHeader from wire format
   */
  void
  wireDecode(const Block& wire);

  /**
   * \brief gets the Source Area field
   */
  std::string
  getSA() const;

  /**
   * \brief gets the Destination Area field
   */
  std::string
  getDA() const;

  /**
   * \brief gets the Previous-hop Position field
   */
  std::string
  getPhPos() const;

  /**
   * \brief gets the Root Name field
   */
  std::string
  getRN() const;

  /**
   * \brief gets the Hop Counter field
   */
  uint8_t
  getHopC() const;
  
  /**
   * \brief sets the Source Area field
   * \param SA a string that contains the source area, is always a geo-area
   */
  void
  setSA(std::string SA);

  /**
   * \brief sets the Destination Area field
   * \param DA a string that contains the destination area, is always a geo-area
   */
  void
  setDA(std::string DA);

  /**
   * \brief sets the Previous-hop field
   * \param phPos a string that contains the positon of the previous node in the format x|y|z.
   */
  void
  setPhPos(std::string phPos);

  /**
   * \brief sets the Root Name field
   * \param RN a string that contains the the root name of the associated NDN packet.
   */
  void
  setRN(std::string RN);

  /**
   * \brief sets the Root Name field
   * \param hopC a unsigned int that contains the number of hops that the packet is allowed to give.
   */
  void
  setHopC(uint8_t hopC);


private:
  mutable Block m_wire;
  std::string m_SA;      // source area
  std::string m_DA;      // destination area
  std::string m_phPos;   // previous hop position
  std::string m_RN;      // root name
  uint8_t m_hopC;        // hop count decressing 
};
/**
 * \brief compares each field between the teo inputs.
 * \return true if they are equal.
 */
bool
operator==(const ValHeader& lhs, const ValHeader& rhs);


NDN_CXX_DECLARE_WIRE_ENCODE_INSTANTIATIONS(ValHeader);

}
}
}
#endif