/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_STRATEGY_HPP
#define VAL_STRATEGY_HPP

#include "val-forwarder.hpp"

#include "ns3/mobility-module.h"


namespace ns3 {
namespace ndn {
namespace val {

/**
 * \brief Pure abstract class, is the base of any new forwarding strategy for VAL
 */
class ValStrategy
{
public:
    ValStrategy(ValForwarder& valFwd);

    virtual
    ~ValStrategy();

    /**
     * \brief public method to be invoked upon a match found in IFNT
     * \details This function calls the base class overriten method doAfterIfntHit.
     */
    void
    afterIfntHit(uint64_t faceId, const std::shared_ptr<const ifnt::Entry>& ifntEntry, const ndn::Interest& interest)
    {
        this->doAfterIfntHit(faceId, ifntEntry, interest);
    }

    /**
     * \brief public method to be invoked upon no match found in IFNT
     * \details This function calls the base class overriten method doAfterIfntMiss.
     */
    void
    afterIfntMiss(uint64_t faceId, const ndn::Interest& interest)
    {
        this->doAfterIfntMiss(faceId, interest);
    }

    /**
     * \brief public method to be invoked upon a match found in DFNT
     * \details This function calls the base class overriten method doAfterDfntHit.
     */
    void
    afterDfntHit(uint64_t faceId, const std::shared_ptr<const dfnt::Entry>& dfntEntry, ifnt::ListMatchResult* ifntEntries ,const ndn::Data& data)
    {
        this->doAfterDfntHit(faceId, dfntEntry, ifntEntries, data);
    }

    /**
     * \brief public method to be invoked upon no match found in DFNT
     * \details This function calls the base class overriten method doAfterDfntMiss.
     */
    void
    afterDfntMiss(uint64_t faceId, const ndn::Data& data, ifnt::ListMatchResult* ifntEntries, bool isProducer)
    {
        this->doAfterDfntMiss(faceId, data, ifntEntries, isProducer);
    }

    /**
     * \brief alows for a mobility model to be added
     */
    void
    setMobilityModel(Ptr<ns3::WaypointMobilityModel> mobilityModel)
    {
        BOOST_ASSERT(mobilityModel != nullptr);
        if(mobilityModel != nullptr) {
            m_mobilityModel = mobilityModel;
        }
    }

protected:
    
    /**
     * \brief method invoked by derive class in order to pass ValPacket to ValForwarder 
     */
    void
    sendValPacket(const uint64_t outFaceId, ValPacket& valPkt, time::nanoseconds duration)
    {
        m_valFwd.registerOutgoingValPacket(outFaceId, valPkt, duration);
    }

    /**
     * \brief method invoked by derive class in order get access to F2A 
     */
    const f2a::F2A&
    getF2A()
    {
        return m_valFwd.m_f2a;
    }

    /**
     * \brief method invoked by derive class in order to obtain a geo-area from F2A 
     */
    const std::string
    getGeoArea(uint64_t faceId)
    {
        auto pair = m_valFwd.m_f2a.findByFaceId(faceId);
        if(pair.first) {
            return pair.second->getGeoArea();
        }
        return "0";
    }

    /**
     * \brief method invoked by derive class in order to get the ID of the outgoing face
     * \details The outgoing face is always a ValNetFace, it obtain the ID of the first ValNetFace registered by ValForwarder
     * \todo Support multiple V2V interfaces 
     */
    u_int64_t
    getValNetFaceId()
    {
        auto it = m_valFwd.m_networkFaces.begin();
        return *it;
    }

    Ptr<ns3::WaypointMobilityModel>
    getMobilityModel()
    {
        return m_mobilityModel;
    }    

    virtual void
    doAfterIfntHit(uint64_t faceId, const std::shared_ptr<const ifnt::Entry>& ifntEntry, const ndn::Interest& interest) = 0;

    virtual void
    doAfterIfntMiss(uint64_t faceId, const ndn::Interest& interest) = 0;

    virtual void
    doAfterDfntHit(uint64_t faceId, const std::shared_ptr<const dfnt::Entry>& dfntEntry, ifnt::ListMatchResult* ifntEntries, const ndn::Data& data) = 0;

    virtual void
    doAfterDfntMiss(uint64_t faceId, const ndn::Data& data, ifnt::ListMatchResult* ifntEntries, bool isProducer) = 0;

protected:
    static constexpr long int MIN_INTEREST_WAIT = 32; // in ms
    static constexpr long int MAX_INTEREST_WAIT = 62; // in ms
    static constexpr long int MIN_DATA_WAIT = 0;      // in ms
    static constexpr long int MAX_DATA_WAIT = 30;     // in ms
    static constexpr long int ZERO_WAIT = 0;          // in ms
    static constexpr long int DELAY_IN_NANOS = 40000; // in ns
    static constexpr long int SIGNAL_RANGE = 110;      // in meters
    static constexpr long int AREA_SIZE = 200;        // area is a rectangle 200m x 200m (L x L);
    static constexpr long int MAX_DISTANCE = 5300;    // in meters

private:
    ValForwarder& m_valFwd;
    Ptr<ns3::WaypointMobilityModel> m_mobilityModel;
};



} // namespace val    
} // namespace ndn    
} // namespace ns3

#endif