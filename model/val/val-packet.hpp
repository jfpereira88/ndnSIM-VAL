/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_PACKET_HPP
#define VAL_PACKET_HPP

#include "val-header.hpp"
#include <memory>

namespace ndn {
    class Interest;
    class Data;
} // namespace ndn


namespace ns3 {
namespace ndn {
/**
 * \ingroup ndn
 * \defgroup val Alterations to support VAL
 */
namespace val {

/**
 * \brief ValPacket represents the packets received and sent by VAL.
 * 
 * This class contains the VAL Header and the NDN packet. <br>
 * The objects have 3 types of state. <br>
 * NOT_SET - 0 <br>
 * DATA_SET - 1 <br>
 * INTEREST_SET - 2 <br>
 * Every object is created in NOT_SET state. <br>
 * Upon the usage of function setInterest or setData the state changes.
 * The state can only be changed once.
 */
class ValPacket
{
public:
    static const uint8_t NOT_SET = 0;
    static const uint8_t DATA_SET = 1;
    static const uint8_t INTEREST_SET = 2;

public:
    /** 
     * \brief Constructor
     * \param valH the VAL header
     */
    ValPacket(const ValHeader& valH);
    ~ValPacket();

    /**
     * \brief Sets and Interest packet into the VAL packet
     * \details it only adds an Interest to the VAL packet if the state is NOT_SET,
     * changes the state to INTEREST_SET
     * \param interest the Interest packet
     */
    void
    setInterest(std::shared_ptr<::ndn::Interest> interest);

    /**
     * \brief Sets and Data packet into the VAL packet
     * \details it only adds an Data to the VAL packet if the state is NOT_SET,
     * changes the state to Data_SET
     * \param data the Data packet
     */
    void
    setData(std::shared_ptr<::ndn::Data> data);

    /**
     * \brief gets the Interest from the VAL packet
     */
    const ::ndn::Interest&
    getInterest() const;

    /**
     * \brief gets the Data packet from the VAL packet
     */
    const ::ndn::Data&
    getData() const;

    /**
     * \brief gets the VAL header from the VAL packet
     */
    const ValHeader&
    getValHeader() const;

    /**
     * \brief indicates the state of the VAL packet
     * \details it should be call to know wich method to invoke in order to extract
     * the NDN packet
     * \return the state of the VAL packet
     */
    const uint8_t
    isSet() const; 

    void
    updatePhPos(std::string pos);

private:
    uint8_t m_isSet;
    ValHeader m_valH;
    std::shared_ptr<::ndn::Interest> m_interest;
    std::shared_ptr<::ndn::Data> m_data;
};

/**
 * \brief compares to ValPackets
 * \details first compares the states of the two packets
 * if it is an interest, it compares the nonces
 * if it is a data, it compares the signatures
 * \todo compare all fields in the ValHeader
 * \return true if equal false if not
 */
bool
operator==(const ValPacket& lhs, const ValPacket& rhs);


} // namespace val
} // namespace ndn
} // namespace ns3
#endif