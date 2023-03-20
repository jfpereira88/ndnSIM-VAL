/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-ifnt-entry.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace ifnt {


Entry::Entry(const ValHeader& valHeader, const ::ndn::Interest& interest, uint64_t faceId)
    : m_da(valHeader.getDA())
    , m_sa(valHeader.getSA())
    , m_nonce(interest.getNonce())
    , m_phPos(valHeader.getPhPos())
    , m_hopC(valHeader.getHopC() - 1)
    , m_rn(valHeader.getRN())
    , m_faceId(faceId)
{
}

Entry::~Entry()
{
}

const std::string
Entry::getDA() const 
{
    return m_da;
}

const std::string
Entry::getSA() const 
{
    return m_sa;
}

const uint32_t
Entry::getNonce() const 
{
    return m_nonce;
}

const std::string
Entry::getPhPos() const
{
    return m_phPos;
}

const uint8_t
Entry::getHopC() const
{
    return m_hopC;
}

const std::string
Entry::getRN() const
{
    return m_rn;
}

const uint64_t
Entry::getFaceId() const 
{
    return m_faceId;
}

bool
operator==(const Entry& l_entry, const Entry& r_entry)
{
    return (l_entry.getDA() == r_entry.getDA() &&
            l_entry.getSA() == r_entry.getSA() &&
            l_entry.getNonce() == r_entry.getNonce() && 
            l_entry.getPhPos() == r_entry.getPhPos() &&
            l_entry.getHopC() == r_entry.getHopC() &&
            l_entry.getRN() == r_entry.getRN() &&
            l_entry.getFaceId() == r_entry.getFaceId());
}

} // namespace ifnt
} // namespace val    
} // namespace ndn
} // namespace ns3
