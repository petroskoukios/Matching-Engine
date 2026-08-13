#include "orderbook/orderbook.hpp"

namespace orderbook {

ErrorCode OrderBook::add(const Order& order) {  // rest a limit order in the book
    if (orders_map.contains(order.id)) return ErrorCode::DuplicateId;
    if (order.quantity == 0) return ErrorCode::InvalidQuantity;
    if (order.type == OrderType::Market) return ErrorCode::MarketOrderCannotRest;

    if (order.side == Side::Buy) {
        auto& level = price_levels_bid[order.price];
        auto it = level.insert(level.end(), order);
        orders_map[order.id] = it;
    } else {
        auto& level = price_levels_ask[order.price];
        auto it = level.insert(level.end(), order);
        orders_map[order.id] = it;
    }
    return ErrorCode::Ok;
}

ErrorCode OrderBook::cancel(OrderID id) {  // remove entirely
    auto index_it = orders_map.find(id);
    if (index_it == orders_map.end()) return ErrorCode::InvalidId;

    auto order_it = index_it->second;
    Order order = *order_it;

    switch (order.side) {
        case Side::Buy: {
            auto& level = price_levels_bid.at(order.price);
            level.erase(order_it);
            if (level.empty()) price_levels_bid.erase(order.price);
            break;
        }
        case Side::Sell: {
            auto& level = price_levels_ask.at(order.price);
            level.erase(order_it);
            if (level.empty()) price_levels_ask.erase(order.price);
            break;
        }
    }

    orders_map.erase(index_it);
    return ErrorCode::Ok;
}

std::optional<Price> OrderBook::best_bid() const {  // highest resting buy
    if (price_levels_bid.empty()) return std::nullopt;
    return price_levels_bid.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {  // lowest resting sell
    if (price_levels_ask.empty()) return std::nullopt;
    return price_levels_ask.begin()->first;
}

Quantity OrderBook::quantity_at(Side side, Price price) const {
    Quantity quantity = 0;
    if (side == Side::Buy) {
        auto it = price_levels_bid.find(price);
        if (it == price_levels_bid.end()) return 0;
        const auto& level = it->second;
        for (const auto& order : level) quantity += order.quantity;
    } else {
        auto it = price_levels_ask.find(price);
        if (it == price_levels_ask.end()) return 0;
        const auto& level = it->second;
        for (const auto& order : level) quantity += order.quantity;
    }
    return quantity;
}

}  // namespace orderbook