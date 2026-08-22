#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "orderbook/orderbook.hpp"

namespace orderbook {

class ReferenceBook {
   public:
    ErrorCode submit(const Order& order, std::vector<Trade>& trades) {
        if (order.quantity == 0) return ErrorCode::InvalidQuantity;
        if (contains_id(order.id)) return ErrorCode::DuplicateId;

        Order incoming = order;
        match(incoming, trades);

        if (incoming.type == OrderType::Limit && incoming.quantity != 0) {
            if (incoming.side == Side::Buy)
                bids_.push_back(incoming);
            else
                asks_.push_back(incoming);
        }

        return ErrorCode::Ok;
    }

    ErrorCode cancel(OrderId id) {
        for (std::size_t i = 0; i < bids_.size(); ++i) {
            if (bids_[i].id == id) {
                bids_.erase(bids_.begin() + i);
                return ErrorCode::Ok;
            }
        }

        for (std::size_t i = 0; i < asks_.size(); ++i) {
            if (asks_[i].id == id) {
                asks_.erase(asks_.begin() + i);
                return ErrorCode::Ok;
            }
        }

        return ErrorCode::InvalidId;
    }

    std::optional<Price> best_bid() const {
        if (bids_.empty()) return std::nullopt;
        Price best = bids_[0].price;
        for (const Order& bid : bids_) best = std::max(best, bid.price);
        return best;
    }

    std::optional<Price> best_ask() const {
        if (asks_.empty()) return std::nullopt;
        Price best = asks_[0].price;
        for (const Order& ask : asks_) best = std::min(best, ask.price);
        return best;
    }

    Quantity quantity_at(Side side, Price price) const {
        Quantity quantity = 0;
        const std::vector<Order>& orders = side == Side::Buy ? bids_ : asks_;
        for (const Order& order : orders) {
            if (order.price == price) quantity += order.quantity;
        }
        return quantity;
    }

   private:
    bool contains_id(OrderId id) const {
        for (const Order& bid : bids_)
            if (bid.id == id) return true;
        for (const Order& ask : asks_)
            if (ask.id == id) return true;
        return false;
    }

    void match(Order& incoming, std::vector<Trade>& trades) {
        if (incoming.side == Side::Buy)
            match_buy(incoming, trades);
        else
            match_sell(incoming, trades);
    }

    void match_buy(Order& incoming, std::vector<Trade>& trades) {
        while (incoming.quantity != 0) {
            const std::optional<std::size_t> index = best_crossing_ask(incoming);
            if (!index) return;
            execute_trade(incoming, asks_, *index, trades);
        }
    }

    void match_sell(Order& incoming, std::vector<Trade>& trades) {
        while (incoming.quantity != 0) {
            const std::optional<std::size_t> index = best_crossing_bid(incoming);
            if (!index) return;
            execute_trade(incoming, bids_, *index, trades);
        }
    }

    std::optional<std::size_t> best_crossing_ask(const Order& incoming) const {
        std::optional<std::size_t> best;
        for (std::size_t i = 0; i < asks_.size(); ++i) {
            const Order& ask = asks_[i];
            if (incoming.type == OrderType::Limit && ask.price > incoming.price) continue;
            if (!best || ask.price < asks_[*best].price) best = i;
        }
        return best;
    }

    std::optional<std::size_t> best_crossing_bid(const Order& incoming) const {
        std::optional<std::size_t> best;
        for (std::size_t i = 0; i < bids_.size(); ++i) {
            const Order& bid = bids_[i];
            if (incoming.type == OrderType::Limit && bid.price < incoming.price) continue;
            if (!best || bid.price > bids_[*best].price) best = i;
        }
        return best;
    }

    static void execute_trade(Order& incoming, std::vector<Order>& resting_orders,
                              std::size_t resting_index, std::vector<Trade>& trades) {
        Order& resting = resting_orders[resting_index];
        const Quantity quantity = std::min(incoming.quantity, resting.quantity);
        trades.push_back(Trade{
            .resting_order_id = resting.id,
            .aggressor_order_id = incoming.id,
            .price = resting.price,
            .quantity = quantity,
        });
        incoming.quantity -= quantity;
        resting.quantity -= quantity;

        if (resting.quantity == 0) resting_orders.erase(resting_orders.begin() + resting_index);
    }

    std::vector<Order> bids_;
    std::vector<Order> asks_;
};

}  // namespace orderbook