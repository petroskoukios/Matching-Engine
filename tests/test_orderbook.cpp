#include <gtest/gtest.h>

#include "orderbook/orderbook.hpp"
#include "orderbook/types.hpp"

class OrderBookTest : public ::testing::Test {
   protected:
    orderbook::OrderBook book;

    static orderbook::Order make_order(orderbook::OrderId id, orderbook::Price price = 100,
                                       orderbook::Quantity qty = 5,
                                       orderbook::Side side = orderbook::Side::Buy,
                                       orderbook::OrderType type = orderbook::OrderType::Limit) {
        return {.id = id, .price = price, .quantity = qty, .side = side, .type = type};
    }
};

TEST_F(OrderBookTest, AddReturnsOkAndOrderIsVisible) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(OrderBookTest, DuplicateIdRejectedAndBookUnchanged) {
    auto order = make_order(1, 100, 10, orderbook::Side::Buy);

    ASSERT_EQ(book.add(order), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.add(order), orderbook::ErrorCode::DuplicateId);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 10);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(OrderBookTest, ZeroQuantityRejectedAndBookUnchanged) {
    EXPECT_EQ(book.add(make_order(1, 100, 0, orderbook::Side::Buy)),
              orderbook::ErrorCode::InvalidQuantity);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST_F(OrderBookTest, MarketOrderRejectedAndNoGhostLevelCreated) {
    EXPECT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy, orderbook::OrderType::Market)),
              orderbook::ErrorCode::MarketOrderCannotRest);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST_F(OrderBookTest, EmptyBookHasNoBestBidOrAsk) {
    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(OrderBookTest, BestBidIsHighestPrice) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 105, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(3, 98, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 105);
}

TEST_F(OrderBookTest, BestAskIsLowestPrice) {
    ASSERT_EQ(book.add(make_order(1, 110, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 105, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(3, 115, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_ask(), 105);
}

TEST_F(OrderBookTest, QuantityAtSumsOrdersAtSamePrice) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 100, 10, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(3, 100, 7, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 22);
}

TEST_F(OrderBookTest, QuantityAtSumsSellOrdersAtSamePrice) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 100, 10, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(3, 100, 7, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 22);
}

TEST_F(OrderBookTest, QuantityAtEmptyPriceReturnsZero) {
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 0);
}

TEST_F(OrderBookTest, UnknownIdRejectedAndBookUnchanged) {
    ASSERT_EQ(book.add(make_order(10, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(999), orderbook::ErrorCode::InvalidId);
    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 5);
}

TEST_F(OrderBookTest, CancelSucceedsAndQuantityDrops) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 10);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 5);

    EXPECT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
}

TEST_F(OrderBookTest, CancelOneOfTwoLevelSurvives) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 100);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(OrderBookTest, CancelLastAtBestNewBestBid) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 99, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 100);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_bid(), 99);
}

TEST_F(OrderBookTest, CancelEverythingNoGhostLevels) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 99, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.cancel(2), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(OrderBookTest, CancelSameIdTwiceSecondReturnsInvalidId) {
    ASSERT_EQ(book.add(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::InvalidId);
}

TEST_F(OrderBookTest, CancelLastSellOrderRemovesLevel) {
    ASSERT_EQ(book.add(make_order(1, 105, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(book.add(make_order(2, 110, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_ask(), 110);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 105), 0);
}