/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-f2a-entry.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace f2a {


Entry::Entry(uint64_t faceId, std::string geoArea)
    : m_faceId(faceId)
    , m_geoArea(geoArea)
{
}

Entry::~Entry()
{
}

const uint64_t
Entry::getFaceId() const
{
    return m_faceId;
}

const std::string
Entry::getGeoArea() const
{
    return m_geoArea;
}

bool
operator==(const Entry& l_entry, const Entry& r_entry)
{
    return (l_entry.getFaceId() == r_entry.getFaceId() && 
            l_entry.getGeoArea() == r_entry.getGeoArea());
}


} // namespace f2a
} // namespace val
} // namespace ndn
} // namespace ns3