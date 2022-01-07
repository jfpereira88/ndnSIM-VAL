/**
 * jfp 2019
 * mieti - uminho
 */
#include "val-strategy.hpp"

namespace ns3 {
namespace ndn {
namespace val {


ValStrategy::ValStrategy(ValForwarder& valFwd)
    : m_valFwd(valFwd)
{
}

ValStrategy::~ValStrategy() = default;

    
} // namespace val    
} // namespace ndn    
} // namespace ns3