/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-packet.hpp"
#include "ns3/log.h"

#include <ndn-cxx/data.hpp>
#include <ndn-cxx/interest.hpp>

NS_LOG_COMPONENT_DEFINE("ndn.val.ValPacket");

namespace ns3 {
namespace ndn {
namespace val {

ValPacket::ValPacket(const ValHeader& valH)
    : m_isSet(ValPacket::NOT_SET)
    , m_valH(valH)
    , m_interest(nullptr)
    , m_data(nullptr)
{
}

ValPacket::~ValPacket()
{
}


void
ValPacket::setInterest(std::shared_ptr<::ndn::Interest> interest)
{
    if(m_isSet == ValPacket::NOT_SET && interest != nullptr) {
        m_interest = interest;
        m_isSet = ValPacket::INTEREST_SET;
    } else {
        NS_LOG_ERROR("Is already set with: " << m_isSet);
    }
}

void
ValPacket::setData(std::shared_ptr<::ndn::Data> data) 
{
    if(m_isSet == ValPacket::NOT_SET && data != nullptr) {
        m_data = data;
        m_isSet = ValPacket::DATA_SET;
    } else {
        NS_LOG_ERROR("Is already set with: " << m_isSet);
    }
}

const ::ndn::Interest&
ValPacket::getInterest() const
{
    return *m_interest;
}

const ::ndn::Data&
ValPacket::getData() const 
{
    return *m_data;
}

const ValHeader&
ValPacket::getValHeader() const
{
    return m_valH;
}

const uint8_t
ValPacket::isSet() const
{
    return m_isSet;
}

void
ValPacket::updatePhPos(std::string pos)
{
    m_valH.setPhPos(pos);
}

bool
operator==(const ValPacket& lhs, const ValPacket& rhs)
{
    if (lhs.isSet() != rhs.isSet()) {
        return false;
    } else if (lhs.isSet() == rhs.isSet() &&  lhs.isSet() == ValPacket::INTEREST_SET) {
        return lhs.getValHeader() == rhs.getValHeader() &&
                lhs.getInterest().getNonce() == rhs.getInterest().getNonce();
    } else if (lhs.isSet() == rhs.isSet() &&  lhs.isSet() == ValPacket::DATA_SET) {
        return lhs.getValHeader() == rhs.getValHeader() &&
                lhs.getData().getSignature() == rhs.getData().getSignature();
    } else {
        return lhs.isSet() == rhs.isSet();  //ValPacket::NOT_SET -> true
    }
}

} // namespace val
} // namespace ndn
} // namespace ns3