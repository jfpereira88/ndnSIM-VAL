/**
 * jfp 2019
 * mieti - uminho
 */

#include "val-geoface-factory.hpp"
#include "val-dummy-link-service.hpp"
#include "../../null-transport.hpp"

namespace ns3 {
namespace ndn {
namespace val {
namespace face {


GeofaceFactory::GeofaceFactory(ValForwarder& valFwd)
    : m_valFwd(valFwd)
{
}

GeofaceFactory::~GeofaceFactory()
{
}

std::shared_ptr<nfd::face::Face>
GeofaceFactory::makeGeoface(){
    auto dummyLink = make_unique<ValDummyLinkService>(m_valFwd);
    auto transport = make_unique<NullTransport>("geoFace://", "geoFace://",
                                              ::ndn::nfd::FACE_SCOPE_NON_LOCAL);
    auto face = std::make_shared<Face>(std::move(dummyLink), std::move(transport));
    face->setGeoFace();
    face->setMetric(1);
    return face;
}


    
} // namespace face
} // namespace val
} // namespace ndn
} // namespace ns3
