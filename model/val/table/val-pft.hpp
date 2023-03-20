/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_PFT_HPP
#define VAL_PFT_HPP

#include "val-pft-entry.hpp"

namespace ns3 {
namespace ndn {
namespace val {
class ValPacket;
namespace pft {

using Table = std::list<std::shared_ptr<Entry>>;
using ListMatchResult = std::vector<std::shared_ptr<Entry>>;

/**
 * \brief PFT - Pending Forwarding Table.
 * \details Registers ValPackets that are to be sent to the network.
 */
class PFT
{    
public:

    /**
     * \brief Constructor - inicializes the list used to store entries.
     */
    PFT();
    ~PFT();

    /**
     * \brief adds an PftEntry to the end of the list
     * \param entry a PftEntry
     * \return a pair of values: first element is True if the entry was added,
     * the second element is a shared pointer to the added entry 
     */
    std::pair<bool, std::shared_ptr<Entry>>
    addEntry(Entry& entry);

    /**
     * \brief finds a PftEntry
     * \details compares the element received with every element in the list
     * comparison made using the == operator of the ValPacket class
     * \param valPkt a ValPacket instance 
     * \return a pair of values: when found returns True, as the first element, and
     * a shared pointer to the entry. When no match is found, the first element is False
     * and the second is a null pointer. 
     */
    std::pair<bool, std::shared_ptr<Entry>>
    findMatch(const ValPacket& valPkt);

    /**
     * \brief finds all the PftEntries that match elements on the receive nonceList
     * \param nonceList a pointer to a vector of nonces
     * \return ListMatchResult a vector of shared pointer to PftEntries
     */
    ListMatchResult
    findMatchByNonceList(std::vector<uint32_t> *nonceList);

    /**
     * \brief Removes a entry
     * \param valPkt a ValPacket that will be used to find a match and remove the entry
     * \return True if a entry was removed, False otherwise
     */
    bool
    removeEntry(const ValPacket& valPkt);

    /**
     * \brief removes all the entries that match the received LisMatchResult
     * \param entries a vector of shared pointer to PftEntries
     */
    void
    removeEntriesByMatchList(ListMatchResult& entries)
    {
        for(const auto& entry : entries) {
            removeEntry(entry->getValPacket());
        }
    }

    /**
     * \brief gets the number o elements in the table
     */
    const size_t
    getSize() const; 

private:
    Table m_pft;
    size_t m_nItens;
};







} // namespace pft
} // namespace val
} // namespace ndn
} // namespace ns3

#endif