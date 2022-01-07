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
 * \details Each object can have one of two states <br>
 * WAITING_FORWARDING - 0 <br>
 * WAITING_IMPLICIT_ACK - 1 <br>
 * This two states control if the entry is ilegable for forwarding cancellation 
 * or implicit acknowlegdement
 */
class Entry
{
friend class PFT;

public:
    // state
    static const int WAITING_FORWARDING = 0;
    static const int WAITING_IMPLICIT_ACK = 1;
    // tries
    static const int FORWARDING_TRIES = -1;
    static const int MAX_IMPLICIT_ACK_TRIES = 3;
    // static const time::milliseconds m_IMPLICIT_ACK_TIME = 100_ms;

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
    getDefaultImpAckTimerDuration() const;

    //sets
    /**
     * \brief Changes the state from WAITING_FORWARDING - 0 to WAITING_IMPLICIT_ACK - 1 <br>
     * \details No other state change is allowed
     */
    bool
    changeStateToWaintingImpAck();

    /**
     * \brief decresses the number of tries by one
     */
    bool
    oneLessTry();

    /**
     * \brief associates and event ID with the entry
     */
    void
    setTimer(::nfd::scheduler::EventId timerId);

private:
    std::shared_ptr<ValPacket> m_valPkt;
    int m_state;
    int m_tries;
    uint64_t m_faceId;
    ::nfd::scheduler::EventId m_timerId;
    // this should be static, but the compiler complains
    const time::milliseconds m_IMPLICIT_ACK_TIME_INTEREST = 70_ms;
    const time::milliseconds m_IMPLICIT_ACK_TIME_DATA = 40_ms;

};



} // namespace pft
} // namespace val
} // namespace ndn
} // namespace ns3

#endif