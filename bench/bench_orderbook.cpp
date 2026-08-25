#include <benchmark/benchmark.h>
#include <random>
#include "orderbook/orderbook.hpp"

static void BM_SubmitMixedStream(benchmark::State& state) {
    std::mt19937 rng;
    std::vector<orderbook::Order> stream;
    int order_id = 1;
    std::uniform_int_distribution<int> price_dist(95, 105);
    std::uniform_int_distribution<int> quantity_dist(1, 10);
    std::uniform_int_distribution<int> limit_or_market(1, 10);
    std::uniform_int_distribution<int> sell_or_buy(1,2);

    for (int i = 0; i < 20000; i++) {
        orderbook::Order order{};
        order.id = order_id++;
        order.quantity = quantity_dist(rng);
        if (limit_or_market(rng) == 1) {
            order.type = orderbook::OrderType::Market;
            order.price = 0;
        }
        else {
            order.type = orderbook::OrderType::Limit;
            order.price = price_dist(rng);
        }
        if (sell_or_buy(rng) == 1) order.side = orderbook::Side::Buy;
        else order.side = orderbook::Side::Sell;
        stream.push_back(order);
    }

    for (auto _ : state) {
        orderbook::OrderBook book;
        std::vector<orderbook::Trade> trades;
        trades.reserve(1024);
        for (const auto& order : stream) {
            benchmark::DoNotOptimize(book.submit(order, trades));
        }
        benchmark::ClobberMemory();
    }
    state.SetItemsProcessed(state.iterations() * stream.size());
}
BENCHMARK(BM_SubmitMixedStream);
