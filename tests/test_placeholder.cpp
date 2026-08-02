#include <gtest/gtest.h>

#include "orderbook/orderbook.hpp"

TEST(EngineTest, GreetingReportsWiring) {
    orderbook::Engine engine;
    EXPECT_EQ(engine.greeting(), "matching-engine: build wiring OK");
}
