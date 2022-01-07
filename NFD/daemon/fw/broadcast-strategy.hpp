#ifndef NFD_DAEMON_FW_BROADCAST_STRATEGY_HPP
#define NFD_DAEMON_FW_BROADCAST_STRATEGY_HPP

#include "strategy.hpp"

namespace nfd {
namespace fw {

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
};

} // namespace fw
} // namespace nfd

#endif // NFD_DAEMON_FW_BROADCAST_STRATEGY_HPP
