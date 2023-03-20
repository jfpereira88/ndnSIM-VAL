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
    , m_state(Entry::FWD)
    , m_tries(Entry::ZERO_RTX_TRIES)
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
Entry::getTimerDelay() const 
{
    time::milliseconds delay = time::milliseconds (0);
    switch (m_state)
    {
    case Entry::HLT:
        delay = time::milliseconds (Entry::HLT_CLEAN_DELAY);
        break;

    case Entry::IMP_ACK:
        delay = time::milliseconds (Entry::IMP_ACK_CLEAN_DELAY);
        break;

    case Entry::RTX:
        if(m_valPkt->isSet() == ValPacket::DATA_SET)
            delay = time::milliseconds (Entry::DATA_RTX_DELAY + (m_tries * Entry::RTX_PENALTY));
        else
            delay = time::milliseconds (Entry::INTEREST_RTX_DELAY + ( m_tries * Entry::RTX_PENALTY));
        break;
    
    default:
        break;
    }
   return delay;
}

const int
Entry::changeState(uint8_t hopCnet, bool sent)
{
    switch(m_state)
    {
    case Entry::FWD:
        if (sent)
        {
            m_state = Entry::RTX;
            if(m_valPkt->isSet() == ValPacket::INTEREST_SET)
              m_tries = Entry::INT_RTX_TRIES;
            else
               m_tries = Entry::DATA_RTX_TRIES;
        }
        else if (hopCnet == getHopC())
        {
            m_state = Entry::HLT;
            m_tries = Entry::HTL_COUNTER;
        }
        /*else
        {
            m_state = Entry::IMP_ACK;
            m_tries = Entry::ZERO_RTX_TRIES;
        }*/
        break;

    case Entry::HLT:
        if (hopCnet == getHopC() && getDA() == "0") // >=??
        {
            m_tries = m_tries - 1;
            if (m_tries == 0)
            {
                m_state = Entry::RTX;
                m_tries = Entry::INT_RTX_TRIES;
            }
        }
        else if (hopCnet < getHopC())
        {
            m_state = Entry::IMP_ACK;
            m_tries = Entry::ZERO_RTX_TRIES;
        }
        /*else
        {
            m_state = Entry::ERROR;
            m_tries = Entry::ZERO_RTX_TRIES;
        }*/
        break;

    case Entry::RTX:
        if (hopCnet < getHopC())
        {
            m_state = Entry::IMP_ACK;
            m_tries = Entry::ZERO_RTX_TRIES;
        }
        break;

    default:
        m_state = Entry::ERROR;
        m_tries = Entry::ZERO_RTX_TRIES;
        break;
    }
   
    return m_state;
}

void
Entry::updatePhPos(std::string pos)
{
    m_valPkt->updatePhPos(pos);
}


u_int8_t
Entry::getHopC()
{
    return m_valPkt->getValHeader().getHopC();
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

const std::string
Entry::getDA()
{
  return m_valPkt->getValHeader().getDA();
}


} // namespace pft
} // namespace val
} // namespace ndn
} // namespace ns3
