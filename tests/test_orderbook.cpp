#include <gtest/gtest.h>

#include "orderbook/orderbook.hpp"
#include "orderbook/types.hpp"

TEST(OrderBookTest, AddReturnsOkAndOrderIsVisible) {
    orderbook::Order order{.id = 1,
                           .price = 100,
                           .quantity = 5,
                           .side = orderbook::Side::Buy,
                           .type = orderbook::OrderType::Limit};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST(OrderBookTest, DuplicateIdRejectedAndBookUnchanged) {
    orderbook::Order order{.id = 1,
                           .price = 100,
                           .quantity = 10,
                           .side = orderbook::Side::Buy,
                           .type = orderbook::OrderType::Limit};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order), orderbook::ErrorCode::DuplicateId);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 10);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST(OrderBookTest, ZeroQuantityRejectedAndBookUnchanged) {
    orderbook::Order order{.id = 1,
                           .price = 100,
                           .quantity = 0,
                           .side = orderbook::Side::Buy,
                           .type = orderbook::OrderType::Limit};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order), orderbook::ErrorCode::InvalidQuantity);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST(OrderBookTest, MarketOrderRejectedAndNoGhostLevelCreated) {
    orderbook::Order order{.id = 1,
                           .price = 100,
                           .quantity = 5,
                           .side = orderbook::Side::Buy,
                           .type = orderbook::OrderType::Market};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order), orderbook::ErrorCode::MarketOrderCannotRest);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST(OrderBookTest, EmptyBookHasNoBestBidOrAsk) {
    orderbook::OrderBook book;

    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST(OrderBookTest, BestBidIsHighestPrice) {
    orderbook::Order order1{.id = 1,
                            .price = 100,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};

    orderbook::Order order2{.id = 2,
                            .price = 105,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};

    orderbook::Order order3{.id = 3,
                            .price = 98,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order2), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order3), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 105);
}

TEST(OrderBookTest, BestAskIsLowestPrice) {
    orderbook::Order order1{.id = 1,
                            .price = 110,
                            .quantity = 5,
                            .side = orderbook::Side::Sell,
                            .type = orderbook::OrderType::Limit};

    orderbook::Order order2{.id = 2,
                            .price = 105,
                            .quantity = 5,
                            .side = orderbook::Side::Sell,
                            .type = orderbook::OrderType::Limit};

    orderbook::Order order3{.id = 3,
                            .price = 115,
                            .quantity = 5,
                            .side = orderbook::Side::Sell,
                            .type = orderbook::OrderType::Limit};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order2), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order3), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_ask(), 105);
}

TEST(OrderBookTest, QuantityAtSumsOrdersAtSamePrice) {
    orderbook::Order order1{.id = 1,
                            .price = 100,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};

    orderbook::Order order2{.id = 2,
                            .price = 100,
                            .quantity = 10,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};

    orderbook::Order order3{.id = 3,
                            .price = 100,
                            .quantity = 7,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};

    orderbook::OrderBook book;

    EXPECT_EQ(book.add(order1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order2), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.add(order3), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 22);
}

TEST(OrderBookTest, QuantityAtEmptyPriceReturnsZero) {
    orderbook::OrderBook book;

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 0);
}