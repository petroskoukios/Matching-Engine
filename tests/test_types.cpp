#include <gtest/gtest.h>

#include <type_traits>

#include "orderbook/types.hpp"

// Compile time layout contracts
static_assert(sizeof(orderbook::Order) == 24, "Order layout changed - was this intentional?");
static_assert(std::is_trivially_copyable_v<orderbook::Order>);
static_assert(std::is_trivially_copyable_v<orderbook::Trade>);
static_assert(sizeof(orderbook::Side) == 1);

// Runtime tests
TEST(TypesTest, OrderConstructsWithDesignatedInitializers) {
    orderbook::Order order{.id = 1,
                           .price = 100,
                           .quantity = 5,
                           .side = orderbook::Side::Buy,
                           .type = orderbook::OrderType::Limit};
    EXPECT_EQ(order.price, 100);
}
