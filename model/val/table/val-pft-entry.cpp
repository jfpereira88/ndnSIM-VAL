/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-pft-entry.hpp"
#include "../val-packet.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace pft {


Entry::Entry(ValPacket&& valPkt, uint64_t faceId)
    : m_valPkt(std::make_shared<ValPacket>(valPkt))
    , m_state(Entry::WAITING_FORWARDING)
    , m_tries(Entry::FORWARDING_TRIES)
    , m_faceId(faceId)
{
}

Entry::~Entry()
{
}

const ValPacket&
Entry::getValPacket() const 
{
    return *m_valPkt;
}

const int
Entry::getState() const 
{
    return m_state;
}

const int
Entry::getNumberOfTries() const 
{
    return m_tries;
}

const uint64_t
Entry::getFaceId() const 
{
    return m_faceId;
}

const ::nfd::scheduler::EventId
Entry::getTimerId() const 
{
    return m_timerId;
}

const time::milliseconds
Entry::getDefaultImpAckTimerDuration() const 
{
    if(m_valPkt->isSet() == ValPacket::DATA_SET)
        return m_IMPLICIT_ACK_TIME_DATA;
    return m_IMPLICIT_ACK_TIME_INTEREST;
}

bool
Entry::changeStateToWaintingImpAck() 
{
    if(m_state == Entry::WAITING_FORWARDING) {
        m_state = Entry::WAITING_IMPLICIT_ACK;
        m_tries = Entry::MAX_IMPLICIT_ACK_TRIES;
        return true;
    }
    return false;
}

bool
Entry::oneLessTry() 
{
    if(m_tries > 0) {
        m_tries--;
        return true;
    }
    return false;
}

void
Entry::setTimer(::nfd::scheduler::EventId timerId) 
{
    m_timerId = timerId;
}


} // namespace pft
} // namespace val
} // namespace ndn
} // namespace ns3