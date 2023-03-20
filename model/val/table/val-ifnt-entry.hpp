/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_TABLE_IFNT_ENTRY_HPP
#define VAL_TABLE_IFNT_ENTRY_HPP

#include <ndn-cxx/interest.hpp>
#include <string>

#include "../val-header.hpp"


namespace ns3 {
namespace ndn {
namespace val {
namespace ifnt {
/**
 * \brief This class represents a IFNT Entry 
 * \details A IFNT Entry contains all the fields of the ValHeader and the nonce of
 * Interest packet, as well as the incoming face ID
 */
class Entry
{

public:
    /**
     * \brief Constructor
     * \param valHeader the ValHeader
     * \param interest the Interest packet
     * \param faceId the id of the incoming face
     * \details The fields of valHeader are safe within member variables, as it is the
     * nonce of the interst.<br>
     * The field hopC in ValHeader is decremented upon creation of the IFNT Entry instance
     */
    Entry(const ValHeader& valHeader, const ::ndn::Interest& interest, uint64_t faceId);
    ~Entry();

    /**
     * \brief gets the Destination Area stored in the entry
     */
    const std::string
    getDA() const;

    /**
     * \brief gets the Source Area stored in the entry
     */
    const std::string
    getSA() const;

    /**
     * \brief gets the Interest nonce stored in the entry
     */
    const uint32_t
    getNonce() const;

    /**
     * \brief gets the Previous-hop position stored in the entry
     */
    const std::string
    getPhPos() const;

    /**
     * \brief gets the Hop counter stored in the entry
     */
    const uint8_t
    getHopC() const;

    /**
     * \brief gets the Root name stored in the entry
     */
    const std::string
    getRN() const;

    /**
     * \brief gets the incoming face id stored in the entry
     */    
    const uint64_t
    getFaceId() const;

private:
    std::string m_da;
    std::string m_sa;        // source area
    uint32_t m_nonce;        // nonce
    std::string m_phPos;     // previous hop position
    uint8_t m_hopC;          // hop count
    std::string m_rn;        // root name
    uint64_t m_faceId;
};

/**
 * \brief This operator returns True if all the member fields of the instances are equal 
 */
bool
operator==(const Entry& l_entry, const Entry& r_entry);

} // namespace ifnt
} // namespace val    
} // namespace ndn
} // namespace ns3



#endif