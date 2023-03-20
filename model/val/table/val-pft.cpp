/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-pft.hpp"
#include "../val-packet.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace pft {

PFT::PFT()
    : m_nItens(0)
{
}

PFT::~PFT()
{
    m_pft.clear();
}

// add or find
std::pair<bool, std::shared_ptr<Entry>>
PFT::addEntry(Entry& entry)
{
    auto found = findMatch(entry.getValPacket());
    if(!found.first) {
        std::shared_ptr<Entry> ptr_entry = std::make_shared<Entry>(entry);
        m_pft.push_back(ptr_entry);
        m_nItens++;
        return {true, ptr_entry};
    }
    return {false, found.second}; //found;
}


// it is equal if it has the same nonce, in the case of being an Interest Pkt
// it is equal if it has the same signature, in the case of being a Data Pkt
std::pair<bool, std::shared_ptr<Entry>>
PFT::findMatch(const ValPacket& valPkt)
{
    
    for(auto it = m_pft.begin(); it != m_pft.end(); it++) {
        // it needs to be the same ndn type
        if(valPkt.isSet() == (*it)->getValPacket().isSet()) {
            if(valPkt.isSet() == ValPacket::INTEREST_SET) {
                if(valPkt.getInterest().getNonce() == (*it)->getValPacket().getInterest().getNonce() &&
                   valPkt.getInterest().getName() == (*it)->getValPacket().getInterest().getName()) {
                    return {true, *it};
                }
            } else if(valPkt.isSet() == ValPacket::DATA_SET) {
                if(valPkt.getData().getSignature() == (*it)->getValPacket().getData().getSignature() &&
                   valPkt.getData().getName () == (*it)->getValPacket().getData().getName()){
                    return {true, *it};
                }
            }
        }
    }
    return {false, nullptr};
}

ListMatchResult
PFT::findMatchByNonceList(std::vector<uint32_t> *nonceList)
{
    ListMatchResult res;
    for(auto it = m_pft.begin(); it != m_pft.end(); it++) {
        // it needs to be Interest
        if((*it)->getValPacket().isSet() == ValPacket::INTEREST_SET) {
            for(const auto &nonce : *nonceList) {
                if((*it)->getValPacket().getInterest().getNonce() == nonce) {
                    res.push_back(*it);
                    //m_pft.erase(it);
                }
            }
        }
        if(res.size() == nonceList->size())
            break;
    }
    return res;
}

bool
PFT::removeEntry(const ValPacket& valPkt)
{
   
    bool state = false;
    for( auto it = m_pft.begin(); it != m_pft.end(); it++) {
        // it needs to be the same ndn type
        if(valPkt.isSet() == (*it)->getValPacket().isSet()) {
            if(valPkt.isSet() == ValPacket::INTEREST_SET) {
                if(valPkt.getInterest().getNonce() == (*it)->getValPacket().getInterest().getNonce()) {
                    m_pft.erase(it);
                    state = true;
                    m_nItens--;
                    it = m_pft.end();
                }
            } else if(valPkt.isSet() == ValPacket::DATA_SET) {
                if(valPkt.getData().getSignature() == (*it)->getValPacket().getData().getSignature()){
                    m_pft.erase(it);
                    state = true;
                    m_nItens--;
                    it = m_pft.end();
                }
            }
        }
    }
    return state;
}

const size_t
PFT::getSize() const 
{
    return m_nItens;
}

} // namespace pft
} // namespace val
} // namespace ndn
} // namespace ns3
