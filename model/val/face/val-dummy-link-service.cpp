/**
 * jfp 2019
 * mieti - uminho
 */
#include "val-dummy-link-service.hpp"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("ndn.val.face.ValDummyLinkService");

namespace ns3 {
namespace ndn {
namespace val {
namespace face {
    
ValDummyLinkService::ValDummyLinkService(ValForwarder& valFwd)
    : m_valFwd(&valFwd)
{
}

ValDummyLinkService::~ValDummyLinkService()
{
}

void
ValDummyLinkService::doSendDataToVal(const Data& data, std::vector<uint32_t> *nonceList, bool isProducer)
{
    BOOST_ASSERT(m_valFwd != nullptr);
    NS_LOG_DEBUG("Sending Data to ValFwd " << data.getName() << 
                " : via face " << this->getFace()->getId() << 
                " is Geoface? " << this->getFace()->isGeoFace());
    m_valFwd->reveiceData(this->getFace(), data, nonceList, isProducer);
}

void
ValDummyLinkService::doSendInterestToVal(const Interest& interest)
{
    NS_LOG_DEBUG("Sending Interest to ValFwd " << interest.getName() << 
                " : via face " << this->getFace()->getId() << 
                " is Geoface? " << this->getFace()->isGeoFace());
    BOOST_ASSERT(m_valFwd != nullptr);
    m_valFwd->reveiceInterest(this->getFace(), interest);
}


// override pure virtual methods
void
ValDummyLinkService::doSendInterest(const Interest& interest)
{
    NS_LOG_ERROR("ValDummyLinkService::doSendInterest -> This should never happen!!!");
    BOOST_ASSERT(false);
}

void
ValDummyLinkService::doSendData(const Data& data)
{
    NS_LOG_ERROR("ValDummyLinkService::doSendData -> This should never happen!!!");
    BOOST_ASSERT(false);
}

void
ValDummyLinkService::doSendNack(const lp::Nack& nack)
{
    NS_LOG_ERROR("ValDummyLinkService::doSendNack -> This should never happen!!!");
    BOOST_ASSERT(false);
}

void
ValDummyLinkService::doReceivePacket(Transport::Packet&& packet)
{
    NS_LOG_ERROR("ValDummyLinkService::doReceivePacket -> This should never happen!!!");
    BOOST_ASSERT(false);
}

} // namespace face
} // namespace val
} // namespace ndn
} // namespace ns3
