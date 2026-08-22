#include <orderbook/types.hpp>
#include <ostream>

namespace orderbook {

std::ostream& operator<<(std::ostream& os, const Trade& trade) {
    return os << "resting_order_id: " << trade.resting_order_id
              << " aggressor_order_id: " << trade.aggressor_order_id << " price: " << trade.price
              << " quantity: " << trade.quantity;
}

}  // namespace orderbook