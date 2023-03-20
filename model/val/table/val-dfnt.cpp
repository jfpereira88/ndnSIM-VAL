/**
 * jfp 2019
 * mieti - uminho
 */

#include"val-dfnt.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace dfnt {

Dfnt::Dfnt()
    :m_nItens(0)
{
}

Dfnt::~Dfnt()
{
    m_table.clear();
}

void
Dfnt::addEntry(Entry& entry)
{
    m_table.push_front(std::make_shared<Entry>(entry));
    m_nItens++;
}

bool 
Dfnt::removeEntry(const Entry& entry)
{
    auto it = m_table.begin();
    while (it != m_table.end()) {
        if (**it == entry) {
            m_table.erase(it);
            m_nItens--;
            return true;
        }
        it++;
    }
    return false;
}

bool
Dfnt::removeEntryBySignature(::ndn::Signature sig)
{
    auto it = m_table.begin();
    while(it != m_table.end()) {
        if((*it)->getSignature() == sig) {
            m_table.erase(it);
            m_nItens--;
            return true;
        }
        it++;
    }
    return false;
}

std::pair<bool, std::shared_ptr<Entry>>
Dfnt::findMatch(::ndn::Signature sig)
{
    auto it = m_table.begin();
    while(it != m_table.end()) {
        if((*it)->getSignature() == sig) {
            return {true, *it};
        }
        it++;
    }
    return {false, nullptr};
}

} // namespace dfnt
} // namespace val
} // namespace ndn
} // namespace ns3