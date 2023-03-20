/**
 * jfpereira
 * @EASYRIDE-UM-SP2
 * 09-09-2020
 */

#include "bcvalc-measurements.hpp"

namespace nfd {
namespace fw {
namespace bcvalc {

NFD_LOG_INIT(BcValCMeasurements);

RouteTimerId::RouteTimerId(const FaceId faceId, const scheduler::EventId eventId)
    : m_faceId(faceId)
    , m_timerId(eventId)
{
}

RouteTimerId::~RouteTimerId()
{
    m_timerId.cancel();
    //::nfd::scheduler::cancel(m_timerId);
    //m_timerId = nullptr;
}

const FaceId
RouteTimerId::getFaceId()
{
    return m_faceId;
}

const scheduler::EventId
RouteTimerId::getTimerId()
{
    return m_timerId;
}


void
RouteTimerId::setTimerId(const scheduler::EventId eventId)
{
    //::nfd::scheduler::cancel(m_timerId);
    m_timerId = eventId;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

RouteAvailabilityInfo::RouteAvailabilityInfo(const fib::Entry& fibEntry)
    : m_fibEntry(&fibEntry)
    , m_itens(0)
{
    NFD_LOG_DEBUG("creating route availability info for fibentry: "<< fibEntry.getPrefix().toUri());    

}

const Name&
RouteAvailabilityInfo::getName()
{
    return m_fibEntry->getPrefix();
}

const fib::Entry&
RouteAvailabilityInfo::getFibEntry()
{
    return *m_fibEntry;
}

void
RouteAvailabilityInfo::updateFibEntry(const fib::Entry& fibEntry)
{
    if(m_itens == 0)
        m_fibEntry = &fibEntry;
}


// false when updates
bool
RouteAvailabilityInfo::addRouteExpiration(const FaceId faceId, const scheduler::EventId eventId)
{
    NFD_LOG_DEBUG(__func__);

    bool added = false;
    if(m_itens == 0) {
        m_routeExpirationTable.push_front(new RouteTimerId(faceId, eventId));
        m_itens++;
        added = true;
    } else {
        auto it = std::find_if(m_routeExpirationTable.begin(), m_routeExpirationTable.end(),
        [faceId] (RouteTimerId* route) { return route->getFaceId() == faceId; });
        if(it != m_routeExpirationTable.end()) {
            (*it)->setTimerId(eventId);
        } else { // does not exist
            m_routeExpirationTable.push_front(new RouteTimerId(faceId, eventId));
            m_itens++;
            added = true;
        }
    }

    NFD_LOG_DEBUG("added new route expiration? " << std::boolalpha << added << 
    "\nfaceId: " << faceId << " | eventID: " << eventId <<
    "\n number of routes: " << m_itens);
    return added;
}

bool
RouteAvailabilityInfo::removeRouteExpiration(const FaceId faceId)
{
    NFD_LOG_DEBUG(__func__);

    bool exists = false;
    if(m_itens > 0) {
        auto it = std::find_if(m_routeExpirationTable.begin(), m_routeExpirationTable.end(),
        [faceId] (RouteTimerId* route) { return route->getFaceId() == faceId; });
        if(it != m_routeExpirationTable.end()) {
            exists = true;
            m_routeExpirationTable.erase(it);
            //delete(*it);
            m_itens--;
        }
    }
    NFD_LOG_DEBUG("route expiration removed? " << std::boolalpha << exists << 
    "\nfaceId: " << faceId <<
    "\n number of routes: " << m_itens);
    return exists; 
}


const scheduler::EventId
RouteAvailabilityInfo::getRouteExperationId(const FaceId faceId)
{
    NFD_LOG_DEBUG(__func__);

    scheduler::EventId timerId = nullptr;
    if(m_itens > 0) {
        auto it = std::find_if(m_routeExpirationTable.begin(), m_routeExpirationTable.end(),
        [faceId] (RouteTimerId* route) { return route->getFaceId() == faceId; });
        if(it != m_routeExpirationTable.end()) {
            timerId = (*it)->getTimerId();
            NFD_LOG_DEBUG("returning eventID: " << timerId);
        }
    }
    return timerId;
}

const size_t
RouteAvailabilityInfo::getNumberOfRoutes()
{
    return m_itens;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

BcValCMeasurements::BcValCMeasurements(MeasurementsAccessor& measurements)
    : m_measurements(measurements)
{
}

RouteAvailabilityInfo*
BcValCMeasurements::getOrCreateRouteAvailabilityInfo(const fib::Entry& fibEntry)
{
    NFD_LOG_DEBUG(__func__);
    measurements::Entry* mEntry = m_measurements.get(fibEntry);
    if (mEntry == nullptr) {
        NFD_LOG_ERROR("getOrCreateRouteAvailabilityInfo: No measurement entry for the prefix: " << fibEntry.getPrefix().toUri());
        return nullptr;
    } else {
        m_measurements.extendLifetime(*mEntry, 100_s);
    }
    RouteAvailabilityInfo* routeInfo = mEntry->insertStrategyInfo<RouteAvailabilityInfo>(fibEntry).first;
    if(routeInfo != nullptr)
        routeInfo->updateFibEntry(fibEntry);
    return routeInfo;
}

RouteAvailabilityInfo*
BcValCMeasurements::getRouteAvailabilityInfo(const Name& prefix)
{
    NFD_LOG_DEBUG(__func__);

    measurements::Entry* mEntry = m_measurements.findExactMatch(prefix);
    if (mEntry == nullptr) {
        NFD_LOG_ERROR("getRouteAvailabilityInfo: No measurement entry for the prefix: " << prefix.toUri());
        return nullptr;
    } else {
        m_measurements.extendLifetime(*mEntry, 100_s);
    }
    RouteAvailabilityInfo* routeInfo = mEntry->getStrategyInfo<RouteAvailabilityInfo>();
    //BOOST_ASSERT(routeInfo != nullptr);
    return routeInfo;
}

void
BcValCMeasurements::removeEntry(const fib::Entry& fibEntry)
{
    measurements::Entry* mEntry = m_measurements.findLongestPrefixMatch(fibEntry.getPrefix());
    mEntry->eraseStrategyInfo<RouteAvailabilityInfo>();
}


} // namespace bcvalc
} // namespace fw
}
