/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-header.hpp"
#include "ndn-cxx/encoding/block-helpers.hpp"

namespace ns3 {
namespace ndn {
namespace val {


ValHeader::ValHeader()
    : m_SA("TEST")
    , m_DA("TEST")
    , m_phPos("TEST")
    , m_RN("TEST")
    , m_hopC(0)
{
}

ValHeader::ValHeader(std::string SA, std::string DA, std::string phPos,
                        std::string RN, uint8_t hopC)
    : m_SA(SA)
    , m_DA(DA)
    , m_phPos(phPos)
    , m_RN(RN)
    , m_hopC(hopC)
{
}

ValHeader::ValHeader(const Block& block)
{
  wireDecode(block);
}

template<::ndn::encoding::Tag TAG>
size_t
ValHeader::wireEncode(::ndn::EncodingImpl<TAG>& encoder) const
{
  size_t length = 0;
  length += ::ndn::encoding::prependStringBlock(encoder, ::ndn::lp::tlv::ValHeaderSA, m_SA);
  length += ::ndn::encoding::prependStringBlock(encoder, ::ndn::lp::tlv::ValHeaderDA, m_DA);
  length += ::ndn::encoding::prependStringBlock(encoder, ::ndn::lp::tlv::ValHeaderPhPos, m_phPos);
  length += ::ndn::encoding::prependStringBlock(encoder, ::ndn::lp::tlv::ValHeaderRN, m_RN);
  length += ::ndn::encoding::prependNonNegativeIntegerBlock(encoder, ::ndn::lp::tlv::ValHeaderHopC, m_hopC);
  length += encoder.prependVarNumber(length);
  length += encoder.prependVarNumber(::ndn::lp::tlv::ValHeader);
  return length;
}

NDN_CXX_DEFINE_WIRE_ENCODE_INSTANTIATIONS(ValHeader);

const Block&
ValHeader::wireEncode() const
{

  if (m_wire.hasWire()) {
    return m_wire;
  }

  ::ndn::EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  ::ndn::EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
ValHeader::wireDecode(const Block& wire)
{
  if (wire.type() != ::ndn::lp::tlv::ValHeader) {
    BOOST_THROW_EXCEPTION(Error("expecting ValHeader block"));
  }

  m_wire = wire;
  m_wire.parse();

  Block::element_const_iterator it = m_wire.elements_begin();
  while (it != m_wire.elements_end()) {
    switch (it->type())
    {
    case ::ndn::lp::tlv::ValHeaderSA:
        m_SA = ::ndn::encoding::readString(*it);
        break;
    case ::ndn::lp::tlv::ValHeaderDA:
        m_DA = ::ndn::encoding::readString(*it);
        break;
    case ::ndn::lp::tlv::ValHeaderPhPos:
        m_phPos = ::ndn::encoding::readString(*it);
        break;
    case ::ndn::lp::tlv::ValHeaderRN:
        m_RN = ::ndn::encoding::readString(*it);
        break;
    case ::ndn::lp::tlv::ValHeaderHopC:
        m_hopC = ::ndn::encoding::readNonNegativeInteger(*it);
        break;
    default:
        BOOST_THROW_EXCEPTION(Error("expecting ValHeader subblocks"));  
        break;
    }
    it++;
  }
}


// gets and sets
std::string
ValHeader::getSA() const {
    return m_SA;
}

std::string
ValHeader::getDA() const {
    return m_DA;
}

std::string
ValHeader::getPhPos() const {
    return m_phPos;
}

std::string
ValHeader::getRN() const {
    return m_RN;
}

uint8_t
ValHeader::getHopC() const {
    return m_hopC;
}

void
ValHeader::setSA(std::string SA) 
{
    // TODO: some format control must be done
    m_SA = SA;
}

void
ValHeader::setDA(std::string DA) 
{
    // TODO: some format control must be done
    m_DA = DA;
}

void
ValHeader::setPhPos(std::string phPos) 
{
    // TODO: some format control must be done
    m_phPos = phPos;
}

void
ValHeader::setRN(std::string RN) 
{
    // TODO: some format control must be done
    m_RN = RN;
}

void
ValHeader::setHopC(uint8_t hopC) 
{
  if(hopC <= ValHeader::MAXHOPS)
    m_hopC = hopC;
}

bool
operator==(const ValHeader& lhs, const ValHeader& rhs)
{
    return  lhs.getDA() == rhs.getDA() &&
            lhs.getHopC() == rhs.getHopC() &&
            lhs.getPhPos() == rhs.getPhPos() &&
            lhs.getRN() == rhs.getRN() &&
            lhs.getSA() == rhs.getSA();
}

} // namespace ns3
}
}
