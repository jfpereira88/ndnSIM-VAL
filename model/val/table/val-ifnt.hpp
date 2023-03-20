/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_TABLE_IFNT_HPP
#define VAL_TABLE_IFNT_HPP

#include <boost/noncopyable.hpp>
#include <list>    // better for adding and removing elements
#include <vector>  // faster
#include <memory>  // for smart pointers

#include "val-ifnt-entry.hpp"


namespace ns3 {
namespace ndn {
namespace val {
namespace ifnt{

using Table = std::list<std::shared_ptr<Entry>>;
using ListMatchResult = std::vector<std::shared_ptr<Entry>>;

/** \brief Ifnt - Interest From Network Table, VAL support structure.
 * 
 * Contains information of the VAL header and the nonce of every 
 * Interest that comes from the network. 
 */ 
class Ifnt : boost::noncopyable
{
public:

    /** 
     * \brief Default constructor
        */
    Ifnt();
    ~Ifnt();
    
    /** 
     * \brief Adds entry to IFNT
    *  \param entry a IFNT Entry
    *  \return true for success
    */
    bool
    addEntry(Entry& entry);

    /** 
     * \brief removes entry from IFNT
     *  \param entry a reference to the IFNT Entry to remove
     *  \return true for success
     */ 
    bool
    removeEntry(const Entry& entry);

    /** 
     * \brief removes entry from IFNT by nonce
     *  \param nonce a uint32_t with the nonce value
     *  \return true for success
     */
    bool
    removeEntryByNonce(const uint32_t nonce) 
    {
        auto pair = findMatchByNonce(nonce);
        if(pair.first)
            return removeEntry(*pair.second); //pointed element
        return pair.first;
    }

    /** 
     * \brief removes entries from IFNT by nonce list
     *  \param nonceList a pointer to the list of nonces
     */
    void
    removeEntriesByNonceList(std::vector<uint32_t> *nonceList)
    {
        for(uint32_t nonce : *nonceList) {
            removeEntryByNonce(nonce);
        }
    }

    /** 
     * \brief finds an entry in IFNT
     *  \param entry a reference to the IFNT entry to match
     *  \return pair in which the first member is a shared pointer to the entry
     *          and the second is a boolean refleting the succes of the operation 
     */
    std::pair<bool, std::shared_ptr<Entry>>
    findMatch(const Entry& entry);

    /** \brief find an entry in IFNT by nonce
     *  \param nonce a uint32_t with the nonce value
     *  \return pair in which the first member is a shared pointer to the entry
     *          and the second is a boolean refleting the succes of the operation
     */
    std::pair<bool, std::shared_ptr<Entry>>
    findMatchByNonce(const uint32_t nonce);

    /** \brief finds all the match in the IFNT given a list of nonces
     *  \param nonceList a pointer to the list of nonces
     *  \return a list of INFT entries shared pointers
     */
    ListMatchResult
    findMatchByNonceList(std::vector<uint32_t> *nonceList);

    /**
     *  \brief The size of the table
     */
    inline size_t
    getIfntSize()
    {
        return m_nItens;
    }

private:
    Table m_table;
    size_t m_nItens;
};


} // namespace ifnt
} // namespace val
} // namespace ndn
} // namespace ns3
#endif