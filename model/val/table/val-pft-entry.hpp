/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_PFT_ENTRY_HPP
#define VAL_PFT_ENTRY_HPP

#include "ns3/ndnSIM/NFD/core/scheduler.hpp"


namespace ns3 {
namespace ndn {
namespace val {
class ValPacket;
namespace pft {

using namespace ::ndn::literals::time_literals;
namespace time = ::ndn::time;

/**
 * \brief This class represents the PFT entry
 * \details Each object can have one of these states <br>
 * FWD - 0 <br>
 * HLT - 1 <br>
 * RTX - 2 <br>
 * IMP_ACK - 3 <br>
 * These states control if the entry is ilegable for forwarding cancellation 
 * or implicit acknowlegdement
 */
class Entry
{
friend class PFT;

public:
    // state
    static const int FWD = 0;
    static const int HLT = 1;
    static const int RTX = 2;
    static const int IMP_ACK = 3;
    static const int ERROR = 4; //debug
    // tries
    static const int ZERO_RTX_TRIES = 0;
    static const int INT_RTX_TRIES = 3;
    static const int DATA_RTX_TRIES = 1;
    static const int RTX_PENALTY = 60;
    static const int HTL_COUNTER = 2;
    // times
    static const int INTEREST_RTX_DELAY = 40;
    static const int DATA_RTX_DELAY = 40;
    static const int HLT_CLEAN_DELAY = 300;
    static const int IMP_ACK_CLEAN_DELAY = 200;


public:
    /**
     * \brief Contructor
     * \param valPkt the ValPacket schedule to be sent to the network
     * \param faceId the id of the outgoing interface
     */
    Entry(ValPacket&& valPkt, uint64_t faceId);
    ~Entry();

    // gets
    /**
     * \brief Gets the ValPacket from witihin the entry
     */
    const ValPacket&
    getValPacket() const;

    /**
     * \brief Gets the state of the entry
     */
    const int
    getState() const;

    /**
     * \brief Gets the number of tries that this entry still has to be transmited.
     */
    const int
    getNumberOfTries() const;

    /**
     * \brief Gets the ID of the ooutgoing interface
     */
    const uint64_t
    getFaceId() const;

    /**
     * \brief Gets the ID of the Event associated with the entry
     */
    const ::nfd::scheduler::EventId
    getTimerId() const;

    /**
     *  \brief Gets the time that a ValPacket needs to wait before retransmission
     *  \details This time depends on the type of NDN packet associated with the ValPacket<br>
     *  if a ValPacket contains a Interest packet uses m_IMPLICIT_ACK_TIME_INTEREST<br>
     *  otherwise it uses m_IMPLICIT_ACK_TIME_DATA
     */
    const time::milliseconds
    getTimerDelay() const;


    /**
     * \brief Changes the state <br>
     */
    const int
    changeState(uint8_t hopCnet = 99, bool sent = false);

    u_int8_t
    getHopC();

    /**
     * \brief decresses the number of tries by one
     */
    bool
    oneLessTry();

    void
    updatePhPos(std::string pos);

    /**
     * \brief associates and event ID with the entry
     */
    void
    setTimer(::nfd::scheduler::EventId timerId);

private:
    const std::string
    getDA();

private:
    std::shared_ptr<ValPacket> m_valPkt;
    int m_state;
    int m_tries;
    uint64_t m_faceId;
    ::nfd::scheduler::EventId m_timerId;

};



} // namespace pft
} // namespace val
} // namespace ndn
} // namespace ns3

#endif
