#pragma once
#include <cstdint>
#include <ostream>

namespace orderbook {

using Price = std::int64_t;  // engine speaks ticks (no currency or decimals)

using Quantity = std::uint32_t;

using OrderId = std::uint64_t;

enum class Side : std::uint8_t { Buy, Sell };

enum class OrderType : std::uint8_t { Limit, Market };

struct Order {
    OrderId id;
    Price price;  // price is unspecified for market orders
    Quantity quantity;
    Side side;
    OrderType type;
};

struct Trade {
    OrderId resting_order_id;
    OrderId aggressor_order_id;
    Price price;
    Quantity quantity;
    bool operator==(const Trade&) const = default;
};

inline std::ostream& operator<<(std::ostream& os, const Trade& trade) {
    return os << "resting_order_id: " << trade.resting_order_id
              << " aggressor_order_id: " << trade.aggressor_order_id << " price: " << trade.price
              << " quantity: " << trade.quantity;
}

}  // namespace orderbook