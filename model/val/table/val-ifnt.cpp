/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-ifnt.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace ifnt {

    Ifnt::Ifnt()
        :m_nItens(0)
    {
    }

    Ifnt::~Ifnt()
    {
        m_table.clear();
    }

    bool
    Ifnt::addEntry(Entry& entry)
    {
        auto pair = findMatch(entry);
        if(!pair.first) {
            m_table.push_back(std::make_shared<Entry>(entry));
            m_nItens++;
        }
        return !pair.first;
    }
    
    bool 
    Ifnt::removeEntry(const Entry& entry)
    {
        auto it = m_table.begin();
        while (it != m_table.end())
        {
            if (*it->get() == entry)
            {
                m_table.erase(it);
                m_nItens--;
                return true;
            }
            it++;
        }
        return false;
    }

    std::pair<bool, std::shared_ptr<Entry>>
    Ifnt::findMatch(const Entry& entry)
    {
        for (auto it = m_table.begin(); it != m_table.end(); it++) {
            if(**it == entry)
                return {true, *it};
        }
        return {false, nullptr}; 
    }

    std::pair<bool, std::shared_ptr<Entry>>
    Ifnt::findMatchByNonce(const uint32_t nonce)
    {
        for (auto it = m_table.begin(); it != m_table.end(); it++) {
            if(it->get()->getNonce() == nonce)
                return {true, *it};
        }
        return {false, nullptr};
    }

    ListMatchResult
    Ifnt::findMatchByNonceList(std::vector<uint32_t> *nonceList)
    {
        ListMatchResult result;
        for (uint32_t nonce : *nonceList) {
            auto pair = findMatchByNonce(nonce);
            if(pair.first)
                result.push_back(pair.second);
        }
        return result;
    }


} // namespace ifnt
} // namespcae val
} // namespcace ndn
} // namespcace ns3