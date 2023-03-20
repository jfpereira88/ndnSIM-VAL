/**
 * jfp 2019
 * mieti - uminho
 */
#ifndef VAL_FACE_GEOFACE_FACTORY_HPP
#define VAL_FACE_GEOFACE_FACTORY_HPP

#include "NFD/daemon/face/face.hpp"
#include <memory>

namespace ns3 {
namespace ndn {
namespace val {
class ValForwarder;
namespace face {

/**
 * \brief This class creates the geoFaces
 * \details each geoFace has a ValDummyLink and a NullTransport<br>
 * The ValDummyLink needs to receive a reference to ValForwarder so the constructor of this class also receives it
 */
class GeofaceFactory
{
public:
    static constexpr nfd::face::FaceId FACEID_COMMVALFACE = 222; 

    static const std::string STR_geoFace;
    static const std::string STR_commValFace;
    static const std::string STR_valNetFace;
    static const std::string STR_floodFace;

public:
    GeofaceFactory(ValForwarder& valFwd);
    ~GeofaceFactory();
 
    /**
     * \brief This function creates the geoFaces
     * \note on creation geoFaces have no faceID, only after added to FaceTable is the id defined
     */
    std::shared_ptr<nfd::face::Face>
    makeGeoface();

    /**
     * \brief This function creates the commValFace
     * \note on creation commValFaces have no faceID, only after added to FaceTable is the id defined
     */
    std::shared_ptr<nfd::face::Face>
    makeCommValFace();

    /**
     * \brief This function creates the valNetFace
     * \note on creation valNetFaces have no faceID, only after added to FaceTable is the id defined
     */
    //std::shared_ptr<nfd::face::Face>
    //makeValNetFace();

private:
    ValForwarder& m_valFwd;    
};
    
} // namespace face
} // namespace val
} // namespace ndn
} // namespace ns3

#endif