/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_F2A_ENTRY_HPP
#define VAL_F2A_ENTRY_HPP

#include <string>

namespace ns3 {
namespace ndn {
namespace val {
namespace f2a {

/**
 * \brief represents a F2A Entry
 * \details This class has two member variables the face ID and the geo-area tag<br>
 * this class has get methods for both member variables, but no set methods
 */
class Entry
{
public:
    /**
     * \brief Constructor
     * \param faceId the id of the geoFace
     * \param geoArea a string that represents the geo-area
     */
    Entry(uint64_t faceId, std::string geoArea);
    ~Entry();

    const uint64_t
    getFaceId() const;

    const std::string
    getGeoArea() const;

private:
    uint64_t m_faceId;
    std::string m_geoArea;
};

bool
operator==(const Entry& l_entry, const Entry& r_entry);

} // namespace f2a
} // namespace val
} // namespace ndn
} // namespace ns3


#endif