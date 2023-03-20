/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-f2a.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace f2a {

F2A::F2A()
    :m_size(0)
{
}

F2A::~F2A()
{
    m_f2a.clear();
}

void
F2A::addEntry(Entry& entry)
{
    if(!findEntry(entry)) {
        m_f2a.push_back(std::make_shared<Entry>(entry));
        m_size++;
    }
}

void
F2A::removeEntry(Entry& entry)
{
   for(auto it = m_f2a.begin(); it != m_f2a.end(); it++) {
        if(**it == entry) {
            m_f2a.erase(it);
            m_size--;
        }
    }
}

void
F2A::removeByFaceId(uint64_t faceId)
{
    for(auto it = m_f2a.begin(); it != m_f2a.end(); it++) {
        if((*it)->getFaceId() == faceId) {
            m_f2a.erase(it);
            m_size--;
        }
    }
}

void
F2A::removeByGeoArea(std::string geoArea)
{
    for(auto it = m_f2a.begin(); it != m_f2a.end(); it++) {
        if((*it)->getGeoArea() == geoArea) {
            m_f2a.erase(it);
            m_size--;
        }
    }
}

const bool
F2A::findEntry(Entry& entry)
{
    for(auto it = m_f2a.begin(); it != m_f2a.end(); it++) {
        if(**it == entry)
            return true;
    }
    return false;
}

std::pair<bool, std::shared_ptr<Entry>>
F2A::findByFaceId(uint64_t faceId)
{
    
    for(auto it = m_f2a.begin(); it != m_f2a.end(); it++) {
        if((*it)->getFaceId() == faceId)
            return {true, *it};
    }
    return {false, nullptr};
}

std::pair<bool, std::shared_ptr<Entry>>
F2A::findByGeoArea(std::string geoArea)
{
    
    for(auto it = m_f2a.begin(); it != m_f2a.end(); it++) {
        if((*it)->getGeoArea() == geoArea)
            return {true, *it};
    }
    return {false, nullptr};
}

const size_t
F2A::getSize() const
{
    return m_size;
}

} // namespace f2a
} // namespace val
} // namespace ndn
} // namespace ns3