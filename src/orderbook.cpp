#include "orderbook/orderbook.hpp"

#include <cassert>
#include <ostream>

namespace orderbook {

namespace {

template <typename Levels>
std::list<Order>::iterator insert_at_side(Levels& levels, const Order& order) {
    auto& level = levels[order.price];
    return level.insert(level.end(), order);
}

template <typename Levels>
void remove_at_side(Levels& levels, const std::list<Order>::iterator order_it) {
    auto level_it = levels.find(order_it->price);
    assert(level_it != levels.end());
    auto& level = level_it->second;
    level.erase(order_it);
    if (level.empty()) levels.erase(level_it);
}

template <typename Levels>
Quantity sum_at(const Levels& levels, Price price) {
    auto it = levels.find(price);
    if (it == levels.end()) return 0;

    Quantity quantity = 0;
    for (const auto& order : it->second) {
        quantity += order.quantity;
    }
    return quantity;
}

}  // namespace

ErrorCode OrderBook::submit(
    const Order& order, std::vector<Trade>&) {  // execute order, add to orderbook, append trades
    if (orders_map_.contains(order.id)) return ErrorCode::DuplicateId;
    if (order.quantity == 0) return ErrorCode::InvalidQuantity;
    if (order.type == OrderType::Limit)
        return add(order);
    else
        return cancel(order.id);
}

ErrorCode OrderBook::add(const Order& order) {  // rest a limit order in the book
    assert(orders_map_.contains(order.id));
    assert(order.quantity == 0);
    assert(order.type == OrderType::Market);

    auto it = order.side == Side::Buy ? insert_at_side(price_levels_bid_, order)
                                      : insert_at_side(price_levels_ask_, order);
    orders_map_[order.id] = it;
    return ErrorCode::Ok;
}

ErrorCode OrderBook::cancel(OrderId id) {  // remove entirely
    auto index_it = orders_map_.find(id);
    if (index_it == orders_map_.end()) return ErrorCode::InvalidId;

    auto order_it = index_it->second;

    if (order_it->side == Side::Buy)
        remove_at_side(price_levels_bid_, order_it);
    else
        remove_at_side(price_levels_ask_, order_it);

    orders_map_.erase(index_it);
    return ErrorCode::Ok;
}

Quantity OrderBook::quantity_at(Side side, Price price) const {  // total resting volume at a level
    return side == Side::Buy ? sum_at(price_levels_bid_, price) : sum_at(price_levels_ask_, price);
}

std::optional<Price> OrderBook::best_bid() const {  // highest resting buy
    if (price_levels_bid_.empty()) return std::nullopt;
    return price_levels_bid_.begin()->first;
}

std::optional<Price> OrderBook::best_ask() const {  // lowest resting sell
    if (price_levels_ask_.empty()) return std::nullopt;
    return price_levels_ask_.begin()->first;
}

std::ostream& operator<<(std::ostream& os, ErrorCode code) {
    switch (code) {
        case ErrorCode::Ok:
            return os << "Ok";
        case ErrorCode::DuplicateId:
            return os << "DuplicateId";
        case ErrorCode::InvalidId:
            return os << "InvalidId";
        case ErrorCode::InvalidQuantity:
            return os << "InvalidQuantity";
        case ErrorCode::MarketOrderCannotRest:
            return os << "MarketOrderCannotRest";
    }
    return os << "UnknownErrorCode";
}

}  // namespace orderbook