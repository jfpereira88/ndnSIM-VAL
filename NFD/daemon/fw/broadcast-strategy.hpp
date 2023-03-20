#ifndef NFD_DAEMON_FW_BROADCAST_STRATEGY_HPP
#define NFD_DAEMON_FW_BROADCAST_STRATEGY_HPP

#include "strategy.hpp"
#include "bcvalc-measurements.hpp"

namespace nfd {
namespace fw {
namespace bcvalc {

/** \brief a forwarding strategy that forwards Interest in complience with VAL 
 */
class BroadcastStrategy : public Strategy
{
public:
  explicit
  BroadcastStrategy(Forwarder& forwarder, const Name& name = getStrategyName());

  static const Name&
  getStrategyName();

  void
  afterReceiveInterest(const Face& inFace, const Interest& interest,
                       const shared_ptr<pit::Entry>& pitEntry) override;

  void
  afterContentStoreHit(const shared_ptr<pit::Entry>& pitEntry,
                       const Face& inFace, const Data& data) override;
  
  void
  afterReceiveData(const shared_ptr<pit::Entry>& pitEntry,
                   const Face& inFace, const Data& data) override;
private:
  Face*
  getOutFaceFromFib(const shared_ptr<pit::Entry>& pitEntry, const Face& inFace, const Interest& interest);

  void
  setRouteInCheck(const fib::Entry& fibEntry, Face* outFace);

  void
  removeRouteFromCheck(const fib::Entry& fibEntry, const Face& inFace);

  void
  disableRouteCallBack(const fib::Entry& fibEntry, Face* face);

public:
  static constexpr time::microseconds ROUTE_LIFETIME = 400_ms; 

private:
  BcValCMeasurements m_measurements;
  size_t m_measurementsCreations;
  size_t m_measurementsLookUps;
  Face* m_commValFace;
};

} // namespace bcvalc

using bcvalc::BroadcastStrategy;

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_BROADCAST_STRATEGY_HPP
