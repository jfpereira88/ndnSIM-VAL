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
    GeofaceFactory(ValForwarder& valFwd);
    ~GeofaceFactory();

    /**
     * \brief This function creates the geoFaces
     * \note on creation geoFaces have no faceID, only after added to FaceTable is the id defined
     */
    std::shared_ptr<nfd::face::Face>
    makeGeoface();

private:
    ValForwarder& m_valFwd;    
};
/*
namespace ns3 {
namespace ndn {
namespace val {
namespace face {

class GeofaceFactory
{
public:
    GeofaceFactory();
    ~GeofaceFactory();

    std::shared_ptr<nfd::face::Face>
    makeGeoface();

private:
      
};*/



    
} // namespace face
} // namespace val
} // namespace ndn
} // namespace ns3

#endif