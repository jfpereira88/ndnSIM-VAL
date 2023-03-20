/**
 * jfpereira
 * @EASYRIDE-UM-SP2
 * 09-09-2020
 */

#ifndef NFD_DAEMON_FW_BCVALC_MEASUREMENTS_HPP
#define NFD_DAEMON_FW_BCVALC_MEASUREMENTS_HPP

#include "fw/strategy-info.hpp"
#include "table/measurements-accessor.hpp"

namespace nfd {
namespace fw {
namespace bcvalc {

class RouteTimerId
{
friend class RouteAvailabilityInfo;

public:
    RouteTimerId(const FaceId faceId, const scheduler::EventId eventId);

    ~RouteTimerId();

    const FaceId
    getFaceId();

    const scheduler::EventId
    getTimerId();

    void
    setTimerId(const scheduler::EventId eventId);

private:
    FaceId m_faceId;
    scheduler::EventId m_timerId;
};

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


using RouteExpirationTable = std::list<RouteTimerId*>;
/**
 * \brief stores strategy information about the following relationship <br>
 * fibentry - name - outFace - removal event, outFace must be a geoFace
 */
class RouteAvailabilityInfo : public StrategyInfo
{
public:
    RouteAvailabilityInfo(const fib::Entry& fibEntry);

    static constexpr int
    getTypeId()
    {
        return 1988;  /** \sa https://redmine.named-data.net/projects/nfd/wiki/StrategyInfoType */
    }

    const Name&
    getName();

    const fib::Entry&
    getFibEntry();

    void
    updateFibEntry(const fib::Entry& fibEntry);

    bool
    addRouteExpiration(const FaceId faceId, const scheduler::EventId eventId);

    bool
    removeRouteExpiration(const FaceId faceId);

    const scheduler::EventId
    getRouteExperationId(const FaceId faceId);

    const size_t
    getNumberOfRoutes();

private:
    const fib::Entry* m_fibEntry;
    RouteExpirationTable m_routeExpirationTable;
    size_t m_itens;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** \brief Helper class to retrieve and create strategy measurements
 */
class BcValCMeasurements : noncopyable
{
public:
    explicit
    BcValCMeasurements(MeasurementsAccessor& measurements);

    RouteAvailabilityInfo*
    getOrCreateRouteAvailabilityInfo(const fib::Entry& fibEntry);

    RouteAvailabilityInfo*
    getRouteAvailabilityInfo(const Name& prefix);

    void
    removeEntry(const fib::Entry& fibEntry);

private:
    void
    extendLifetime(measurements::Entry& me);

public:
    static constexpr time::microseconds MEASUREMENTS_LIFETIME = 100_s;

private:
  MeasurementsAccessor& m_measurements;
};


}
}
}

#endif
