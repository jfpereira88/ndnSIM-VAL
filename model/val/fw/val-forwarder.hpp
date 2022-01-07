/**
 * jfp 2019
 * mieti - uminho
 */
#ifndef VAL_FW_FORWARDER_HPP
#define VAL_FW_FORWARDER_HPP


#include "NFD/daemon/fw/face-table.hpp"
#include "NFD/daemon/fw/forwarder.hpp"
#include "NFD/core/scheduler.hpp"

#include "../val-api.hpp"
#include "../val-packet.hpp"
#include "../table/val-ifnt.hpp"
#include "../table/val-dfnt.hpp"
#include "../table/val-pft.hpp"
#include "../table/val-f2a.hpp"
#include "../face/val-geoface-factory.hpp"

#include <list>

namespace ndn {
class Data;
class Interest;
}

namespace ns3 {
namespace ndn {
    class L3Protocol;
namespace val {
    class ValStrategy;

using namespace ::ndn::literals::time_literals;
//namespace time = ::ndn::time;

/**
 * \brief This class represents the forwarding plane in VAL
 */
class ValForwarder
{
friend class ValStrategy;

public:
    /**
     * \brief Constructor
     * \details it receives an L3Protocol instance in order to have access to nfd::FaceTable<br>
     * and to add faces with ndn::Tracers in order to obttain metrics
     */
    ValForwarder(ndn::L3Protocol& l3P);
    ~ValForwarder();

    /**
     * \brief Method that deals with Interests received from NFD
     * \details It is invoked by ValDummyLink<br>
     * It tries to find a match in IFNT and passes the packet to ValStrategy according
     * to the match result.
     * \param inGeoface a pointer to the incoming geoFace
     * \param interest a ref to the Interest
     */
    void
    reveiceInterest(const nfd::Face *inGeoface, const Interest& interest);

    /**
     * \brief Method that deals with Data packets received from NFD
     * \details It is invoked by ValDummyLink<br>
     * It tries to find a match in DFNT and passes the packet to ValStrategy according
     * to the match result.
     * \param inGeoface a pointer to the incoming geoFace
     * \param data a ref to the Data packet
     * \param nonceList a pointer to a vector of nonces
     * \param isProducer a boolean that identifies if the Data was producer by an APP
     */
    void
    reveiceData(const nfd::Face *inGeoface, const Data& data, std::vector<uint32_t> *nonceList, bool isProducer);

    /**
     * \brief This function is used to create and insert entries in PFT
     * \details This function is called from ValStrategy and also controls the creation of forwarding events.
     * \param outFaceId the id of the outgoing face
     * \param valPkt the Valpacket to be sent to the network
     * \param duration the duration of the forwarding timer
     */
    void
    registerOutgoingValPacket(const nfd::FaceId outFaceId, ValPacket& valPkt, time::nanoseconds duration);

private:
    /**
     * \brief This function deals with a new incoming ValPacket
     * \details This function is called when ValForwarder receives, via signal, a ValPacket from ValNetFace
     * \param face the incoming interface
     * \param valP the ValPacket to be treated  
     */
    void
    onReceivedValPacket(const Face& face, const ValPacket& valP);

    /**
     * \brief Treats a Interest extracted from ValPacket that came from the network
     * \details this function creates the IFNT entry, the geoFaces and sends the packet to NFD
     * \param face incoming face
     * \param valH the ValHeader present in the received ValPacket
     * \param interest the Interest present in the received ValPacket
     */
    void
    processInterestFromNetwork(const Face& face, const ValHeader& valH, const Interest& interest);


    /**
     * \brief Treats a Data extracted from ValPacket that came from the network
     * \details this function creates the IFNT entry, the geoFaces and sends the packet to NFD
     * \param face incoming face
     * \param valH the ValHeader present in the received ValPacket
     * \param data the Data present in the received ValPacket
     */
    void
    processDataFromNetwork(const Face& face, const ValHeader& valH, const Data& data);


    /**
     * \brief Removes ValNetFace
     * \details is called if the signal beforeRemoveValFace is detected
     * \todo remove IFNT PFT DFNT related entries
     */
    void
    cleanupOnFaceRemoval(const Face& face);

    /**
     * \brief this function adds a ValNetFace id to the m_networkFaces list
     * \details it is called if the signal afterAddValFace is detected
     */
    void
    addToNetworkFaceList(const Face& face);

    /**
     * \brief this function removes a ValNetFace id to the m_networkFaces list
     */
    void
    removeFromNetworkFaceList(const Face& face);

    /**
     * \brief This function uses m_networkFaces list and the received id to return the requested ValNetFace 
     */
    Face*
    getNetworkFace(nfd::FaceId faceId);

    /**
     * \brief This functions uses m_networkFaces list and the received id to return the first ValNetFace whose id is diferent from the one just received  
     */
    Face*
    getOtherNetworkFace(nfd::FaceId faceId);

    //received packet path
    /**
     * \brief This function uses the PFT table in order to implemente the forwarding cancellation and implicit ack mechanisms
     */
    bool
    preProcessingValPacket(const Face& face, const ValPacket& valP);

    
    // received Interest from network path
    /**
     * \brief This function controls if an Interest packet has left t its destinanion area
     * \todo pass this function to ValStrategy
     */
    bool
    isEscapedPacket(const ValHeader& valH);

    /**
     * \brief This function implements the forwarding timer
     */
    void
    forwardingTimerCallback(const std::shared_ptr<pft::Entry>& pftEntry, const nfd::FaceId outFaceId);


    /**
     * \brief This function implements the retransmission timer
     */
    void
    impAckTimerCallback(const std::shared_ptr<pft::Entry>& pftEntry, const nfd::FaceId outFaceId);

    /** \brief Make geofaces from ValHeader
     * \return first FaceId_SA, second FaceID_DA
     */
    std::pair<uint32_t, uint32_t>
    makeGeoFaceFromValHeader(const ValHeader& valH);

private:
    ndn::L3Protocol *m_l3P; // add faces via this pointer
    nfd::FaceTable *m_faceTable;
    ifnt::Ifnt m_ifnt;
    dfnt::Dfnt m_dfnt;
    pft::PFT m_pft;
    f2a::F2A m_f2a;
    ValStrategy *m_strategy;
    face::GeofaceFactory m_geofaceFactory;
    std::list<nfd::FaceId> m_networkFaces;
    // counters
    uint32_t m_invalidIN;
};

                
} // namespace val            
} // namespace ndn        
} // namespace ns3

#endif