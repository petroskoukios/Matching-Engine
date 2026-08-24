#include <benchmark/benchmark.h>
#include "orderbook/orderbook.hpp"

static void BM_SubmitMixedStream(benchmark::State& state) {
    std::vector<orderbook::Order> stream;
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
