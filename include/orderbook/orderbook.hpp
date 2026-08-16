#pragma once

#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>

#include "orderbook/types.hpp"

namespace orderbook {

enum class ErrorCode : std::uint8_t {
    Ok = 0,
    DuplicateId,
    InvalidId,
    InvalidQuantity,
    MarketOrderCannotRest,
};

std::ostream& operator<<(std::ostream& os, ErrorCode code);

class OrderBook {
   public:
    [[nodiscard]] ErrorCode add(const Order& order);  // rest a limit order in the book
    [[nodiscard]] ErrorCode submit(
        const Order& order,
        std::vector<Trade>& trades);             // execute order, add to orderbook, append trades
    [[nodiscard]] ErrorCode cancel(OrderId id);  // remove entirely
    [[nodiscard]] std::optional<Price> best_bid() const;  // highest resting buy
    [[nodiscard]] std::optional<Price> best_ask() const;  // lowest resting sell
    [[nodiscard]] Quantity quantity_at(Side side,
                                       Price price) const;  // total resting volume at a level

   private:
    std::map<Price, std::list<Order>, std::greater<Price>> price_levels_bid_;
    std::map<Price, std::list<Order>, std::less<Price>> price_levels_ask_;
    std::unordered_map<OrderId, std::list<Order>::iterator> orders_map_;
};

}  // namespace orderbook