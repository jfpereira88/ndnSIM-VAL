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

const std::string GeofaceFactory::STR_geoFace{"geoFace://"};
const std::string GeofaceFactory::STR_commValFace{"commValFace://"};
const std::string GeofaceFactory::STR_valNetFace{"valNetFace://"};
const std::string GeofaceFactory::STR_floodFace{"floodFace://"};

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

std::shared_ptr<nfd::face::Face>
GeofaceFactory::makeCommValFace(){
    auto dummyLink = make_unique<ValDummyLinkService>(m_valFwd);
    auto transport = make_unique<NullTransport>(GeofaceFactory::STR_commValFace, GeofaceFactory::STR_commValFace,
                                              ::ndn::nfd::FACE_SCOPE_LOCAL);
    auto face = std::make_shared<Face>(std::move(dummyLink), std::move(transport));
    face->setMetric(1);
    face->setGeoFace();
    return face;
}



    
} // namespace face
} // namespace val
} // namespace ndn
} // namespace ns3
