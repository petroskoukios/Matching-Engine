#pragma once
#include <cstdint>

namespace orderbook {

using Price = std::int64_t;  // engine speaks ticks (no currency or decimals)

using Quantity = std::uint32_t;

using OrderID = std::uint64_t;

enum class Side : std::uint8_t { Buy, Sell };

enum class OrderType : std::uint8_t { Limit, Market };

struct Order {
    OrderID id;
    Price price;
    Quantity quantity;
    Side side;
    OrderType type;
};

struct Trade {
    OrderID resting_order_id;
    OrderID aggressor_order_id;
    Price price;
    Quantity quantity;
};

}  // namespace orderbook