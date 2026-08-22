#include <gtest/gtest.h>

#include <vector>

#include "orderbook/types.hpp"
#include "reference_book.hpp"

class ReferenceBookTest : public ::testing::Test {
   protected:
    orderbook::ReferenceBook book;

    static orderbook::Order make_order(orderbook::OrderId id, orderbook::Price price = 100,
                                       orderbook::Quantity qty = 5,
                                       orderbook::Side side = orderbook::Side::Buy,
                                       orderbook::OrderType type = orderbook::OrderType::Limit) {
        return {.id = id, .price = price, .quantity = qty, .side = side, .type = type};
    }

    orderbook::ErrorCode submit(const orderbook::Order& order,
                                std::vector<orderbook::Trade>& trades) {
        return book.submit(order, trades);
    }
    orderbook::ErrorCode submit(const orderbook::Order& order) {
        std::vector<orderbook::Trade> trades;
        return book.submit(order, trades);
    }
};

TEST_F(ReferenceBookTest, 2AddReturnsOkAndOrderIsVisible) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(ReferenceBookTest, 2DuplicateIdRejectedAndBookUnchanged) {
    auto order = make_order(1, 100, 10, orderbook::Side::Buy);

    ASSERT_EQ(submit(order), orderbook::ErrorCode::Ok);

    EXPECT_EQ(submit(order), orderbook::ErrorCode::DuplicateId);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 10);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(ReferenceBookTest, 2ZeroQuantityRejectedAndBookUnchanged) {
    EXPECT_EQ(submit(make_order(1, 100, 0, orderbook::Side::Buy)),
              orderbook::ErrorCode::InvalidQuantity);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2EmptyBookHasNoBestBidOrAsk) {
    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2BestBidIsHighestPrice) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 105, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(3, 98, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 105);
}

TEST_F(ReferenceBookTest, 2BestAskIsLowestPrice) {
    ASSERT_EQ(submit(make_order(1, 110, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 105, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(3, 115, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_ask(), 105);
}

TEST_F(ReferenceBookTest, 2QuantityAtSumsOrdersAtSamePrice) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 10, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(3, 100, 7, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 22);
}

TEST_F(ReferenceBookTest, 2QuantityAtSumsSellOrdersAtSamePrice) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 10, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(3, 100, 7, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 22);
}

TEST_F(ReferenceBookTest, 2QuantityAtEmptyPriceReturnsZero) {
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 0);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 0);
}

TEST_F(ReferenceBookTest, 2UnknownIdRejectedAndBookUnchanged) {
    ASSERT_EQ(submit(make_order(10, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(999), orderbook::ErrorCode::InvalidId);
    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 5);
}

TEST_F(ReferenceBookTest, 2CancelSucceedsAndQuantityDrops) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 10);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 5);

    EXPECT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
}

TEST_F(ReferenceBookTest, 2CancelOneOfTwoLevelSurvives) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 100);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_bid(), 100);
}

TEST_F(ReferenceBookTest, 2CancelLastAtBestNewBestBid) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 99, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 100);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_bid(), 99);
}

TEST_F(ReferenceBookTest, 2CancelEverythingNoGhostLevels) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 99, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.cancel(2), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2CancelSameIdTwiceSecondReturnsInvalidId) {
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::InvalidId);
}

TEST_F(ReferenceBookTest, 2CancelLastSellOrderRemovesLevel) {
    ASSERT_EQ(submit(make_order(1, 105, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 110, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.cancel(1), orderbook::ErrorCode::Ok);
    EXPECT_EQ(book.best_ask(), 110);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 105), 0);
}

TEST_F(ReferenceBookTest, 2MarketOrderOnEmptyBookFillsNothingAndDoesNotRest) {
    auto order = make_order(1, 100, 5, orderbook::Side::Buy, orderbook::OrderType::Market);
    std::vector<orderbook::Trade> trades;
    EXPECT_EQ(book.submit(order, trades), orderbook::ErrorCode::Ok);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(book.best_bid(), std::nullopt);  // critically: it did NOT rest
}

TEST_F(ReferenceBookTest, 2ExactMatchProducesOneTradeAndEmptiesBook) {
    std::vector<orderbook::Trade> trades;
    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 5u);
    EXPECT_EQ(trades[0].resting_order_id, 1u);
    EXPECT_EQ(trades[0].aggressor_order_id, 2u);
    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2NonCrossingBuyRestsAndBothSidesPopulated) {
    ASSERT_EQ(submit(make_order(1, 101, 5, orderbook::Side::Sell)), orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(2, 99, 5, orderbook::Side::Buy)), orderbook::ErrorCode::Ok);

    EXPECT_EQ(book.best_bid(), 99);
    EXPECT_EQ(book.best_ask(), 101);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 99), 5);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 101), 5);
}

TEST_F(ReferenceBookTest, 2IncomingLargerBuyConsumesAskAndRestsRemainder) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(2, 100, 8, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 5u);
    EXPECT_EQ(trades[0].resting_order_id, 1u);
    EXPECT_EQ(trades[0].aggressor_order_id, 2u);

    EXPECT_EQ(book.best_ask(), std::nullopt);
    EXPECT_EQ(book.best_bid(), 100);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Buy, 100), 3);
}

TEST_F(ReferenceBookTest, 2RestingLargerAskKeepsRemainingQuantity) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 10, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(2, 100, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].quantity, 5u);

    EXPECT_EQ(book.best_ask(), 100);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 5);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2BuySweepsMultipleAskLevelsInPriceOrder) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 101, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(3, 101, 10, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 2u);

    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 5u);

    EXPECT_EQ(trades[1].price, 101);
    EXPECT_EQ(trades[1].quantity, 5u);

    EXPECT_EQ(book.best_ask(), std::nullopt);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2FIFOAtSamePriceFillsOldestOrderFirst) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(3, 100, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);

    EXPECT_EQ(trades[0].resting_order_id, 1u);
    EXPECT_EQ(trades[0].aggressor_order_id, 3u);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 5);
}

TEST_F(ReferenceBookTest, 2BuyGetsPriceImprovementFromBetterAsk) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(2, 105, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);

    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[0].quantity, 5u);
}

TEST_F(ReferenceBookTest, 2MarketBuySweepsMultipleAskLevels) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 101, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(
        submit(make_order(3, 0, 10, orderbook::Side::Buy, orderbook::OrderType::Market), trades),
        orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].price, 100);
    EXPECT_EQ(trades[1].price, 101);

    EXPECT_EQ(book.best_ask(), std::nullopt);
    EXPECT_EQ(book.best_bid(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2MarketBuyLargerThanLiquidityDoesNotRest) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 10, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(
        submit(make_order(2, 0, 20, orderbook::Side::Buy, orderbook::OrderType::Market), trades),
        orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].quantity, 10u);

    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2SellAggressorConsumesBidAndLeavesRemainder) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(2, 100, 8, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 1u);

    EXPECT_EQ(trades[0].resting_order_id, 1u);
    EXPECT_EQ(trades[0].aggressor_order_id, 2u);
    EXPECT_EQ(trades[0].price, 100);

    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), 100);
    EXPECT_EQ(book.quantity_at(orderbook::Side::Sell, 100), 3);
}

TEST_F(ReferenceBookTest, 2SellSweepsMultipleBidLevelsInPriceOrder) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 101, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 100, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(submit(make_order(3, 100, 10, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    ASSERT_EQ(trades.size(), 2u);

    EXPECT_EQ(trades[0].price, 101);
    EXPECT_EQ(trades[1].price, 100);

    EXPECT_EQ(book.best_bid(), std::nullopt);
    EXPECT_EQ(book.best_ask(), std::nullopt);
}

TEST_F(ReferenceBookTest, 2BookAlwaysMaintainsValidSpreadAfterTrades) {
    std::vector<orderbook::Trade> trades;

    ASSERT_EQ(submit(make_order(1, 100, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);
    ASSERT_EQ(submit(make_order(2, 110, 5, orderbook::Side::Sell), trades),
              orderbook::ErrorCode::Ok);

    if (book.best_bid() && book.best_ask()) {
        EXPECT_LT(*book.best_bid(), *book.best_ask());
    }

    ASSERT_EQ(submit(make_order(3, 110, 5, orderbook::Side::Buy), trades),
              orderbook::ErrorCode::Ok);

    if (book.best_bid() && book.best_ask()) {
        EXPECT_LT(*book.best_bid(), *book.best_ask());
    }
}