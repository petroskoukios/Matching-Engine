#pragma once

#include <algorithm>
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
};

std::ostream& operator<<(std::ostream& os, ErrorCode code);

class OrderBook {
   public:
    [[nodiscard]] ErrorCode submit(
        const Order& order,
        std::vector<Trade>& trades);             // execute order, add to orderbook, append trades
    [[nodiscard]] ErrorCode cancel(OrderId id);  // remove entirely
    [[nodiscard]] std::optional<Price> best_bid() const;  // highest resting buy
    [[nodiscard]] std::optional<Price> best_ask() const;  // lowest resting sell
    [[nodiscard]] Quantity quantity_at(Side side,
                                       Price price) const;  // total resting volume at a level

   private:
    template <typename ContraSideLevels>
    void match(Order& order, ContraSideLevels& contra_side_levels, std::vector<Trade>& trades) {
        while (order.quantity > 0) {
            if (contra_side_levels.empty()) return;

            auto level_it = contra_side_levels.begin();
            std::list<Order>& level = level_it->second;
            Order& resting_order = level.front();
            auto resting_order_id = resting_order.id;
            auto resting_price = resting_order.price;

            if (!crosses(order, resting_price)) return;
            Quantity fill = std::min(order.quantity, resting_order.quantity);
            order.quantity -= fill;
            resting_order.quantity -= fill;

            Trade trade{.resting_order_id = resting_order_id,
                        .aggressor_order_id = order.id,
                        .price = resting_price,
                        .quantity = fill};
            trades.push_back(trade);

            if (resting_order.quantity == 0) {
                orders_map_.erase(resting_order_id);
                level.pop_front();
                if (level.empty()) contra_side_levels.erase(level_it);
            }
        }
    }

    static bool crosses(const Order& order, Price resting_price);
    void add(const Order& order);  // rest a limit order in the book
    std::map<Price, std::list<Order>, std::greater<Price>> price_levels_bid_;
    std::map<Price, std::list<Order>, std::less<Price>> price_levels_ask_;
    std::unordered_map<OrderId, std::list<Order>::iterator> orders_map_;
};

}  // namespace orderbook