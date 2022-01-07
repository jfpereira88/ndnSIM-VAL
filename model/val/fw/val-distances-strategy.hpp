/**
 * jfp 2019
 * mieti - uminho
 */

#ifndef VAL_DISTANCES_STRATEGY_HPP
#define VAL_DISTANCES_STRATEGY_HPP

#include "val-strategy.hpp"

namespace ns3 {
namespace ndn {
namespace val {


/**
 * \brief This class implements a forwarding strategy based on distances to the DA and between nodes
 * 
 */
class ValDistancesStrategy : public ValStrategy
{
public:
    /**
     * \brief Constructor
     * \details receives a reference to ValForwarder in order to satisfy the constructor of the base class
     */
    ValDistancesStrategy(ValForwarder& valFwd);
    ~ValDistancesStrategy();

private:
    /**
     * \brief This method implements the strategy for forwarding Interest that came from the network
     */
    virtual void
    doAfterIfntHit(uint64_t faceId, const std::shared_ptr<const ifnt::Entry>& ifntEntry, const ndn::Interest& interest) override;

    /**
     * \brief implements the startegy for forwarding locally generated Interests
     */
    virtual void
    doAfterIfntMiss(uint64_t faceId, const ndn::Interest& interest) override;

    /**
     * \brief This method implements the strategy for forwarding Data that came from the network
     */
    virtual void
    doAfterDfntHit(uint64_t faceId, const std::shared_ptr<const dfnt::Entry>& dfntEntry, ifnt::ListMatchResult* ifntEntries, const ndn::Data& data) override;

    /**
     * \brief implements the startegy for forwarding locally generated Data
     */
    virtual void
    doAfterDfntMiss(uint64_t faceId, const ndn::Data& data, ifnt::ListMatchResult* ifntEntries, bool isProducer) override;

    /**
     * \brief Generates a delay in microseconds order, converts the delay to nanoseconds and returns it
     */
    time::nanoseconds
    generateMicroSecondDelay();

    /**
     * \brief receives IFNT Entries and returns a list of the phPos member of those Entries
     */
    std::vector<std::string>
    getPositions(ifnt::ListMatchResult* ifntEntriesList);

    /**
     * \brief This function calculates the mean distance between the pointA and a list of points (pointList)
     */
    double
    getMultiPointDist(const std::string pointA, std::vector<std::string> *pointsList);

    /**
     *  \brief calculates the distance froma point to a area
     */
    double
    getDistanceToArea(const std::string pointA, const std::string area);

    /**
     * \brief calculates the distance between to points
     */
    double
    getDistanceToPoint(const std::string pointA, const std::string pointB);

    /**
     * \brief Gets current node position
     */
    std::string
    getMyPos();

    /**
     * \brief Gets current node geo-area
     */
    std::string
    getMyArea();

    /**
     * \brief calculates the timer duration  based on a distance
     * \details it can be used to calculate times for Interests or Data packets <br>
     * times for distances to a geo-area <br>
     * times for distances between nodes <br>
     * follows the less distance less time approach
     * \param dist the distance, it can be between points or between point and geo-area
     * \param hopc number of hops that a packet still has - default value 0
     * \param toArea a boolean that tells the function that the distance is in ralation to a geo-area if set to True
     * \param isData a boolean that tells the function that the timer is for a Data packet if set to True
     */
    time::nanoseconds
    calcFwdTimer(double dist, uint8_t hopC = 0, bool toArea = false, bool isData = false);

    /**
     * \brief calculates the timer duration  based on the inverse distance (1/distance)
     * \details it can be used to calculate times for Interests or Data packets <br>
     * times for distances to a geo-area <br>
     * times for distances between nodes <br>
     * follows the more distance less time approach
     * \param dist the distance, it can be between points or between point and geo-area
     * \param hopc number of hops that a packet still has - default value 0
     * \param toArea a boolean that tells the function that the distance is in ralation to a geo-area if set to True
     * \param isData a boolean that tells the function that the timer is for a Data packet if set to True
     */
    time::nanoseconds
    calcInvertedFwdTimer(double dist, uint8_t hopC = 0, bool toArea = false, bool isData = false);

    /**
     * \brief given the x and y positions returns the correspondent geo-area
     */
    std::string
    getAreaFromPosition(double _x, double _y);

    /**
     * \brief returns the central point of a geo-area
     */
    ns3::Vector3D
    getPositionFromArea(std::string area);

    /**
     *  \brief return the information of the interest that as done the longest jorney
     *  \return first: hopc, second: sourceArea
     */
    std::pair<uint8_t, std::string>
    getLongestJorney(ifnt::ListMatchResult* ifntEntriesList);

private:
    Ptr<UniformRandomVariable> m_randomNum;
};



} // namespace val    
} // namespace ndn    
} // namespace ns3

#endif