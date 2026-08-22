#include <gtest/gtest.h>

#include <random>
#include <string>
#include <vector>

#include "orderbook/orderbook.hpp"
#include "reference_book.hpp"

void expect_agreement(const orderbook::OrderBook& fast, const orderbook::ReferenceBook& ref) {
    EXPECT_EQ(fast.best_bid(), ref.best_bid());
    EXPECT_EQ(fast.best_ask(), ref.best_ask());

    for (orderbook::Price p = 95; p <= 105; ++p) {
        EXPECT_EQ(fast.quantity_at(orderbook::Side::Buy, p),
                  ref.quantity_at(orderbook::Side::Buy, p));
        EXPECT_EQ(fast.quantity_at(orderbook::Side::Sell, p),
                  ref.quantity_at(orderbook::Side::Sell, p));
    }

    if (fast.best_bid() && fast.best_ask()) {
        EXPECT_LT(*fast.best_bid(), *fast.best_ask());
    }
}

TEST(Differential, RandomStreamsAgree) {
    std::uniform_int_distribution<int> roll_dist(0, 99);
    std::uniform_int_distribution<int> price_dist(95, 105);
    std::uniform_int_distribution<int> quantity_dist(1, 10);
    std::uniform_int_distribution<int> side_dist(0, 99);
    std::uniform_int_distribution<int> type_dist(0, 99);

    for (unsigned seed = 0; seed < 100; ++seed) {
        SCOPED_TRACE("seed " + std::to_string(seed));
        std::mt19937 rng(seed);
        orderbook::OrderBook fast;
        orderbook::ReferenceBook ref;
        orderbook::OrderId next_id = 1;
        std::vector<orderbook::OrderId> submitted_ids;

        for (int op = 0; op < 200; ++op) {
            SCOPED_TRACE("operation " + std::to_string(op));

            std::vector<orderbook::Trade> fast_trades;
            std::vector<orderbook::Trade> ref_trades;

            if (roll_dist(rng) < 70) {
                orderbook::Order order{};
                order.id = next_id++;
                order.quantity = quantity_dist(rng);
                order.side = side_dist(rng) < 50 ? orderbook::Side::Buy : orderbook::Side::Sell;
                order.type = type_dist(rng) < 10 ? orderbook::OrderType::Market
                                                 : orderbook::OrderType::Limit;
                order.price = order.type == orderbook::OrderType::Market ? 0 : price_dist(rng);
                const auto fast_result = fast.submit(order, fast_trades);
                const auto ref_result = ref.submit(order, ref_trades);
                EXPECT_EQ(fast_result, ref_result);
                EXPECT_EQ(fast_trades, ref_trades);
                submitted_ids.push_back(order.id);

                for (const auto& trade : fast_trades) {
                    EXPECT_GT(trade.quantity, 0);
                }
                for (const auto& trade : ref_trades) {
                    EXPECT_GT(trade.quantity, 0);
                }
            } else {
                orderbook::OrderId id;

                if (!submitted_ids.empty() && roll_dist(rng) > 10) {
                    std::uniform_int_distribution<std::size_t> index_dist(0,
                                                                          submitted_ids.size() - 1);

                    id = submitted_ids[index_dist(rng)];
                } else {
                    std::uniform_int_distribution<orderbook::OrderId> bogus_id_dist(next_id,
                                                                                    next_id + 30);

                    id = bogus_id_dist(rng);
                }

                const auto fast_result = fast.cancel(id);
                const auto ref_result = ref.cancel(id);

                EXPECT_EQ(fast_result, ref_result);
            }

            expect_agreement(fast, ref);
            if (::testing::Test::HasFailure()) return;
        }
    }
}