/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_FACE_DUMMY_LINK_SERVICE_HPP
#define VAL_FACE_DUMMY_LINK_SERVICE_HPP

#include "NFD/daemon/face/link-service.hpp"
#include "../fw/val-forwarder.hpp"

namespace ns3 {
namespace ndn {
namespace val {
class ValForwarder;
namespace face {

using ::nfd::face::LinkService;
using ::nfd::face::Transport;

/**
 * \brief This class is used as the link layer in geoFaces
 * \details it is reponsable to  NDN packet to VAL directly<br>
 * in order to do that it receives a reference to ValForwarder
 */
class ValDummyLinkService : public LinkService
{
public:
    ValDummyLinkService(ValForwarder& valFwd);
    
    ~ValDummyLinkService() override;

private:
    /**
   * \brief overrides base class method and sends - making it do nothing
   * \note The base class is used to lunch the event that passes the IntPkt to NFD
   */
    void
    doSendInterest(const Interest& interest) override;
    
    /**
    * \brief overrides base class method and sends - making it do nothing
    * \note The base class is used to lunch the event that passes the DataPkt to NFD
    */
    void
    doSendData(const Data& data) override;

    /**
    * \brief overrides base class method and sends - making it do nothing
    */
    void
    doSendNack(const lp::Nack& nack) override;

    /**
    * \brief overrides base class method and sends - making it do nothing
    * \note geoFace don't use Transport layer, therefore there is no communication
    */
    void
    doReceivePacket(Transport::Packet&& packet) override;
    /**
     * \brief function responsable to passe Data packets to VAL
     * \param Data the data packet
     * \param nonceList a pointer to a vector of nonces
     * \param isProducer a boolean to indicate if the node is the data producer
     */
    void
    doSendDataToVal(const Data& data, std::vector<uint32_t> *nonceList, bool isProcucer) override;

    /**
     * \brief send Interests to VAL
     */
    void
    doSendInterestToVal(const Interest& interest) override;


private:
    ValForwarder* m_valFwd;
};



} // namespace face
} // namespace val
} // namespace ndn
} // namespace ns3

#endif