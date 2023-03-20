/**
 * jfp 2019
 * mieti - uminho
 */


#include "val-distances-strategy.hpp"
#include <ndn-cxx/util/random.hpp>
#include <ndn-cxx/lp/tags.hpp>
#include <boost/chrono.hpp>
#include "ns3/log.h"

#include "ns3/vector.h"



NS_LOG_COMPONENT_DEFINE("ndn.val.ValDistancesStrategy");


namespace ns3 {
namespace ndn {
namespace val {


ValDistancesStrategy::ValDistancesStrategy(ValForwarder& valFwd)
    : ValStrategy(valFwd)
    , m_randomNum(CreateObject<UniformRandomVariable>())
{
    NS_LOG_DEBUG("Creating VALStrategy-Distances");
}

ValDistancesStrategy::~ValDistancesStrategy() = default;

void
ValDistancesStrategy::doAfterIfntHit(uint64_t faceId, const std::shared_ptr<const ifnt::Entry>& ifntEntry, const ndn::Interest& interest)
{
    NS_LOG_DEBUG(__func__);
    if(ifntEntry->getHopC() == 0) {
        NS_LOG_DEBUG("packet dropped hopC = 0");
        return; // drop packet
    }
    
    if(ifntEntry->getDA() != "0" && ifntEntry->getDA() != getMyArea()) { // known destination
        // get distance between ifntEntry->getPhPos() and ifntEntry->getDA()
        uint32_t preHopDist = getDistanceToArea(ifntEntry->getPhPos(), ifntEntry->getDA());
        // get distance between getMyPos() and ifntEntry->getDA()
        uint32_t myDist = getDistanceToArea(getMyPos(), ifntEntry->getDA());
        if(myDist < preHopDist) {
        //    construct ValHeader that respects consumer's will
              ValHeader valH(ifntEntry->getSA(), ifntEntry->getDA(), 
                    getMyPos(), ifntEntry->getRN(), ifntEntry->getHopC());
              ValPacket valP(valH);
              valP.setInterest(std::make_shared<Interest>(interest));
        //      calculate the duration of the forwarding timer, less distance less time
              time::nanoseconds time = calcFwdTimer(myDist, ifntEntry->getHopC(), true, false);
              sendValPacket(ifntEntry->getFaceId(), valP, time);
         } else {
        //      drop packet
            NS_LOG_DEBUG("my dist > prevHopDist " << std::to_string(myDist) << " > " << std::to_string(preHopDist));
            NS_LOG_DEBUG("packet dropped");
         }
    } else {  // exploration phase
        // get distance betwwen the current node and the previous node
        uint32_t dist = getDistanceToPoint(ifntEntry->getPhPos(), getMyPos());
        // calculate the duration of the forwarding timer, more distance less time
        time::nanoseconds time = calcInvertedFwdTimer(dist, 0, false, false);
        //std::string destinationArea = this->getGeoArea(faceId);
        ValHeader valH(ifntEntry->getSA(), ifntEntry->getDA(),
                getMyPos(), ifntEntry->getRN(), ifntEntry->getHopC());
        ValPacket valP(valH);
        valP.setInterest(std::make_shared<Interest>(interest));
        sendValPacket(ifntEntry->getFaceId(), valP, time);
    }
    /*
    ValHeader valH(ifntEntry->getSA(), ifntEntry->getDA(), 
                    ifntEntry->getPhPos(), ifntEntry->getRN(), ifntEntry->getHopC());
    ValPacket valP(valH);
    valP.setInterest(std::make_shared<Interest>(interest));
    time::milliseconds time = time::milliseconds{ValDistancesStrategy::MIN_INTEREST_WAIT};
    time = time + time::duration_cast<time::milliseconds>(generateMicroSecondDelay());
    NS_LOG_DEBUG("Timer: " << time);
    sendValPacket(ifntEntry->getFaceId(), valP, time);
    */
}

void
ValDistancesStrategy::doAfterIfntMiss(uint64_t faceId, const ndn::Interest& interest)
{
    NS_LOG_DEBUG(__func__);
    std::string destinationArea = this->getGeoArea(faceId);
    uint8_t hopC = ValHeader::MAXHOPS;
    time::milliseconds time = time::milliseconds{ValDistancesStrategy::ZERO_WAIT};
    ValHeader valH(getMyArea(), destinationArea, getMyPos(), interest.getName().toUri(), hopC);
    ValPacket valP(valH);
    valP.setInterest(std::make_shared<Interest>(interest));
    NS_LOG_DEBUG("Sending Val Packet from Strategy");
    sendValPacket(getValNetFaceId(), valP, time);
    
    /*
    std::string destinationArea = this->getGeoArea(faceId);
    uint8_t hopC = ValHeader::MAXHOPS;
    time::milliseconds time = time::milliseconds{ValDistancesStrategy::MIN_INTEREST_WAIT};
    ValHeader valH("0", destinationArea, 
                "0", interest.getName().toUri(), hopC);
    ValPacket valP(valH);
    valP.setInterest(std::make_shared<Interest>(interest));
    sendValPacket(getValNetFaceId(), valP, time);
    */
}

void
ValDistancesStrategy::doAfterDfntHit(uint64_t faceId, const std::shared_ptr<const dfnt::Entry>& dfntEntry, ifnt::ListMatchResult* ifntEntries, const ndn::Data& data)
{
    NS_LOG_DEBUG(__func__);
    if(dfntEntry->getHopC() == 0) {
        NS_LOG_DEBUG("packet dropped hopC = 0");
        return; // drop packet
    }
    std::vector<std::string> nextHopsPosList = getPositions(ifntEntries);
    uint32_t prevHopDist = getMultiPointDist(dfntEntry->getPhPos(), &nextHopsPosList); //getDistanceToArea(dfntEntry->getPhPos(), dfntEntry->getDA());
    uint32_t myDist = getMultiPointDist(getMyPos(), &nextHopsPosList);//getDistanceToArea(getMyPos(), dfntEntry->getDA());
    //
    if(prevHopDist > ValDistancesStrategy::SIGNAL_RANGE && myDist < prevHopDist) {
        time::nanoseconds time = calcFwdTimer(myDist, 0, false, true);
        ValHeader valH(dfntEntry->getSA(), dfntEntry->getDA(), 
                    getMyPos(), dfntEntry->getRN(), dfntEntry->getHopC());
        ValPacket valP(valH);
        valP.setData(std::make_shared<Data>(data));
        sendValPacket(dfntEntry->getFaceId(), valP, time);
        NS_LOG_DEBUG ("DFNTHIT myDist: " << std::to_string (myDist));
        for(auto it = ifntEntries->begin(); it != ifntEntries->end(); it++) {
            NS_LOG_DEBUG ("My POS: " << getMyPos () << " ifntEntryPos: " << (*it)->getPhPos());
          }
    } else {
        if(myDist > prevHopDist) {
            NS_LOG_DEBUG("my dist > prevHopDist " << std::to_string(myDist) << " > " << std::to_string(prevHopDist)
            << " ifntEntries size " << std::to_string(ifntEntries->size()));
        } else {
            NS_LOG_DEBUG("prevHopDist < SIGNAL_RANGE prev-hop already served the next-hop " << std::to_string(prevHopDist) << " < " << std::to_string(ValDistancesStrategy::SIGNAL_RANGE)
            << " ifntEntries size " << std::to_string(ifntEntries->size()));
        }
        NS_LOG_DEBUG("packet dropped");
    }
    // else drop packet
}

void
ValDistancesStrategy::doAfterDfntMiss(uint64_t faceId, const ndn::Data& data, ifnt::ListMatchResult* ifntEntries, bool isProducer)
{
    NS_LOG_DEBUG(__func__);
    //auto pair = getLongestJorney(ifntEntries);
    uint8_t hopc = ValHeader::MAXHOPS;
    uint8_t extra_hops = 2;
    std::string srcArea;
    auto it = ifntEntries->begin();
    while(it != ifntEntries->end()) {
        hopc = ValHeader::MAXHOPS - (*it)->getHopC() + extra_hops; // three more hops for good luck
        //hopc = 1;
        if (hopc > ValHeader::MAXHOPS)
            hopc = ValHeader::MAXHOPS;
        if(isProducer) {
            srcArea = getMyArea();
        } else {
            srcArea = this->getGeoArea(data.getCongestionMark());
            if(srcArea == "0") {
              srcArea = getMyArea();
              NS_LOG_DEBUG ("DA was Zero");
            }
            data.removeTag<lp::CongestionMarkTag>();
        }
        ValHeader valH(srcArea, (*it)->getSA(), 
                    getMyPos(), data.getName().toUri(), hopc);
        ValPacket valP(valH);
        valP.setData(std::make_shared<Data>(data));
        time::milliseconds time = time::milliseconds{ValDistancesStrategy::ZERO_WAIT};
        sendValPacket(getValNetFaceId(), valP, time);

        it++;
    }

    
    // @REMEMBER: Data Last hop does not receive ImpACK
}

time::nanoseconds
ValDistancesStrategy::generateMicroSecondDelay()
{
    NS_LOG_DEBUG(__func__);
    //long int random = ::ndn::random::generateWord32();
    //random = random / (ValDistancesStrategy::MAX_32WORD_RANDOM/ValDistancesStrategy::DELAY_IN_MICROS);
    long int random = m_randomNum->GetValue(10000, ValDistancesStrategy::DELAY_IN_NANOS);
    return time::nanoseconds{random};
}

std::vector<std::string>
ValDistancesStrategy::getPositions(ifnt::ListMatchResult* ifntEntriesList)
{
    NS_LOG_DEBUG(__func__);
    std::vector<std::string> res;
    for(auto it = ifntEntriesList->begin(); it != ifntEntriesList->end(); it++) {
        res.push_back((*it)->getPhPos());
    }
    return res;
}


double
ValDistancesStrategy::getMultiPointDist(const std::string pointA, std::vector<std::string> *pointsList)
{
    NS_LOG_DEBUG(__func__);
    size_t nItens = pointsList->size();
    //NS_LOG_DEBUG("size of ifnt entries" << std::to_string(nItens));
    // geting first vector;
    std::stringstream stream_point;
    stream_point << pointA;
    ns3::Vector3D vectorA;
    stream_point >> vectorA;

    // reset stream
    // stream_point.str(std::string());

    double dist = 0.0;
    for(const std::string& point : *pointsList) {
        NS_LOG_DEBUG("Reference point: " << pointA);
        NS_LOG_DEBUG("Interest came from: " << point);
        std::stringstream stream_point_temp;
        stream_point_temp << point;
        ns3::Vector3D vector;
        stream_point_temp >> vector;
        dist += ns3::CalculateDistance(vectorA, vector);
        // reset stream
        // stream_point.str(std::string());
    }
    // calculate the mean
    NS_LOG_DEBUG("total dist: " << std::to_string(dist));
    dist = dist / double(nItens);
    return dist;
}

double
ValDistancesStrategy::getDistanceToArea(const std::string pointA, const std::string area)
{
    NS_LOG_DEBUG(__func__);
    std::stringstream stream_pointA;
    stream_pointA << pointA;
    ns3::Vector3D vectorA;
    stream_pointA >> vectorA;
    return ns3::CalculateDistance(vectorA, getPositionFromArea(area));
}

double
ValDistancesStrategy::getDistanceToPoint(const std::string pointA, const std::string pointB)
{
    NS_LOG_DEBUG(__func__);
    std::stringstream stream_pointA;
    std::stringstream stream_pointB;
    stream_pointA << pointA;
    stream_pointB << pointB;

    ns3::Vector3D vectorA;
    ns3::Vector3D vectorB;

    stream_pointA >> vectorA;
    stream_pointB >> vectorB;
    
    return ns3::CalculateDistance(vectorA, vectorB);
}

std::string
ValDistancesStrategy::getMyPos()
{
    NS_LOG_DEBUG(__func__);
    std::stringstream stream;
    ns3::Vector3D myPosVector = this->getMobilityModel()->GetPosition();
    stream << myPosVector;
    return stream.str();
}

std::string
ValDistancesStrategy::getMyArea()
{
    NS_LOG_DEBUG(__func__);
    ns3::Vector3D myPosVector = this->getMobilityModel()->GetPosition();
    // check X and Y agains the pseudo AreaTable
    return getAreaFromPosition(myPosVector.x, myPosVector.y);
}

time::nanoseconds
ValDistancesStrategy::calcFwdTimer(double dist, uint8_t hopC, bool toArea, bool isData)
{ 
    NS_LOG_DEBUG(__func__);
    // less distance less time;
    //long int dist = long(dist);
    double time = 0.0;
    double max_dist = double(ValDistancesStrategy::SIGNAL_RANGE);
    double time_gap = 0.0;  // range of time
    long int min_time = 0.0;  // minimum waiting time
  
    if(isData) {
        time_gap = double(ValDistancesStrategy::MAX_DATA_WAIT - ValDistancesStrategy::MIN_DATA_WAIT);
        min_time = ValDistancesStrategy::MIN_DATA_WAIT * 1000000; //to nano
    } else { 
        time_gap = double(ValDistancesStrategy::MAX_INTEREST_WAIT - ValDistancesStrategy::MIN_INTEREST_WAIT);
        min_time = ValDistancesStrategy::MIN_INTEREST_WAIT * 1000000;  // to nano
    }

    if(toArea) {
        BOOST_ASSERT(hopC != 0); // is the dist calculated to an Area?: it needs hopC
        max_dist = double(hopC * ValDistancesStrategy::SIGNAL_RANGE);
        if(dist > max_dist) {
            NS_LOG_DEBUG("calcFwdTimer dist > max_dist: " << std::to_string(dist) << " > " << 
                std::to_string(max_dist) << " time: " << std::to_string(time) << 
                " isData: " << std::boolalpha << isData << " toArea: " << std::boolalpha << toArea);
                long int long_time_gap(time_gap);
            return time::nanoseconds{min_time} + time::nanoseconds{long_time_gap} + this->generateMicroSecondDelay();
        }
    }

    time = dist / (max_dist/time_gap); // this time is in milliseconds
    time *= 1000000; // to nano
    long int long_time(time);
    NS_LOG_DEBUG("calcFwdTimer dist: " << std::to_string(dist) << " time: " << std::to_string(time) << 
        " isData: " << std::boolalpha << isData << " toArea: " << std::boolalpha << toArea);
    
    // convertions to get time in nano seconds in the following maner
    // min_time + time + random
    return time::nanoseconds{min_time} + time::nanoseconds{long_time} + this->generateMicroSecondDelay();
}

time::nanoseconds
ValDistancesStrategy::calcInvertedFwdTimer(double dist, uint8_t hopC, bool toArea, bool isData)
{
    NS_LOG_DEBUG(__func__);
    // more distance less time;
    double time = 0.0;
    double max_dist = double(ValDistancesStrategy::SIGNAL_RANGE);
    double time_gap = 0.0;  // range of time
    long int min_time = 0;  // minimum waiting time
    
    if(isData) {
        time_gap = double(ValDistancesStrategy::MAX_DATA_WAIT - ValDistancesStrategy::MIN_DATA_WAIT);
        min_time = ValDistancesStrategy::MIN_DATA_WAIT * 1000000; //to nano;
    } else { 
        time_gap = double(ValDistancesStrategy::MAX_INTEREST_WAIT - ValDistancesStrategy::MIN_INTEREST_WAIT);
        min_time = ValDistancesStrategy::MIN_INTEREST_WAIT * 1000000; //to nano;
    }
    
    if(toArea) {
        BOOST_ASSERT(hopC != 0); // is the dist calculated to an Area?: it needs hopC
        max_dist = double(hopC * ValDistancesStrategy::SIGNAL_RANGE);
        if(dist > max_dist) {
            NS_LOG_DEBUG("calcInvertedFwdTimer dist > max_dist: " << std::to_string(dist) << " > " << 
                std::to_string(max_dist) << " time: " << std::to_string(time) << 
                " isData: " << std::boolalpha << isData << " toArea: " << std::boolalpha << toArea);
            return time::nanoseconds{min_time} + time::nanoseconds{min_time} + this->generateMicroSecondDelay();
        }
    }
    if(dist <= max_dist)
        time = time_gap - ((time_gap/max_dist) * dist); // this time is in milliseconds
    else
        time = double(min_time);
    time *= 1000000; // to nano
    long int long_time(time);
    NS_LOG_DEBUG("calcInvertedFwdTimer dist: " << std::to_string(dist) << " time: " << std::to_string(time) << 
        " isData: " << std::boolalpha << isData << " toArea: " << std::boolalpha << toArea);
    
    // convertions to get time in nano seconds in the following maner
    // min_time + time + random
    return time::nanoseconds{min_time} + time::nanoseconds{long_time} + this->generateMicroSecondDelay();
}

std::pair<uint8_t, std::string>
ValDistancesStrategy::getLongestJorney(ifnt::ListMatchResult* ifntEntriesList)
{
    NS_LOG_DEBUG(__func__);
    //NS_LOG_DEBUG("node at: " << getMyPos());
    auto it = ifntEntriesList->begin();
    uint8_t min = 0;
    std::string srcArea = "zz";
    if(*it != nullptr) { 
        min = (*it)->getHopC();
        srcArea = (*it)->getSA(); 
        it++;  // next
        while(it != ifntEntriesList->end()) {
            if(*it != nullptr) {
                if(min > (*it)->getHopC()) {
                    min = (*it)->getHopC();
                    srcArea = (*it)->getSA();
                }
            }
            it++;
        }
    }
    return {min, srcArea};
}

std::string
ValDistancesStrategy::getAreaFromPosition(double _x, double _y)
{
    NS_LOG_DEBUG(__func__);
    double temp_x = _x / ValDistancesStrategy::AREA_SIZE;
    double temp_y = _y / ValDistancesStrategy::AREA_SIZE;

    // round down (truncate) doubles
    int area_x_step = int(temp_x);
    int area_y_step = int(temp_y);

    std::stringstream stream;

    char origin_x = 'A';  // ASCII 65
    char origin_y = '1';  // ASCII 49
    // set bounderies
    if(area_x_step > 25)
        origin_x = 'Z';
    else
        origin_x += area_x_step;
    
    if(area_y_step > 5)
        origin_y = '5';
    else
        origin_y += area_y_step;
        
        
    stream << origin_x << origin_y;
    return stream.str();
}

ns3::Vector3D
ValDistancesStrategy::getPositionFromArea(std::string area)
{
    NS_LOG_DEBUG(__func__);
    auto it = area.begin();
    int first = *it;
    int second = *++it;
    double dist_x = 100.0;  // default area A1
    double dist_y = 100.0;  // default area A1
    int step_x = first - 65;  // 65 ASCII A
    int step_y = second - 49; // 49 ASCII 1

    // the positon of the middle of the area
    if(step_x >= 0) {
        dist_x = (ValDistancesStrategy::AREA_SIZE * (step_x +1)) - (ValDistancesStrategy::AREA_SIZE / 2);
    }
    if(step_y >= 0) {
        dist_y = (ValDistancesStrategy::AREA_SIZE * (step_y +1)) - (ValDistancesStrategy::AREA_SIZE / 2);
    }
    return ns3::Vector3D(dist_x, dist_y, 0.0);
}

void
ValDistancesStrategy::doSendZeroData(const std::shared_ptr<const dfnt::Entry>& dfntEntry, const ndn::Data& data)
{
   /* if (dfntEntry->getHopC () <= 1){
      NS_LOG_DEBUG ("Returned hopC = " << std::to_string (dfntEntry->getHopC ()));
      return;
    }*/
    ValHeader valH(dfntEntry->getSA(), dfntEntry->getDA(), 
                    getMyPos(), dfntEntry->getRN(), 1);
    ValPacket valP(valH);
    valP.setData(std::make_shared<Data>(data));
    time::milliseconds time = time::milliseconds{ValDistancesStrategy::ZERO_WAIT};
    sendValPacket(dfntEntry->getFaceId(), valP, time);
}

void
ValDistancesStrategy::doUnsolisitedData (const std::shared_ptr<const dfnt::Entry>& dfntEntry, const ndn::Data& data)
{
  // needs a time:
  //    the closer it is to the DA the smaller the wait - small dist small timer - normaltimer
  //    the further away it is from the precious hop the smaller the wait - big dist small timer -inverted timer
  std::string rnUnsolicided = dfntEntry->getRN();
  rnUnsolicided.append (" Unsolicited");
  ValHeader valH(dfntEntry->getSA(), dfntEntry->getDA(),
                    getMyPos(), rnUnsolicided, dfntEntry->getHopC ());
  ValPacket valP(valH);
  valP.setData(std::make_shared<Data>(data));
  time::nanoseconds time = time::nanoseconds(0);
  if (dfntEntry->getDA() != getMyArea())
  {
    uint32_t preHopDist = getDistanceToArea(dfntEntry->getPhPos(), dfntEntry->getDA());
    uint32_t myDist = getDistanceToArea(getMyPos(), dfntEntry->getDA());
    if (myDist < preHopDist)
    {
      time = calcFwdTimer(myDist, dfntEntry->getHopC(), true, true);
    }
    else
    {
      return;
    }
  }
  else
  {
    uint32_t dist = getDistanceToPoint(dfntEntry->getPhPos(), getMyPos());
    time = calcInvertedFwdTimer(dist, dfntEntry->getHopC(), false, true);
  }
   time = time +  time::nanoseconds(30000000); // + 30 millis
   sendValPacket(dfntEntry->getFaceId(), valP, time);
}

} // namespace val    
} // namespace ndn    
} // namespace ns3
