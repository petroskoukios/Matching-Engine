#include "orderbook/orderbook.hpp"

#include <gtest/gtest.h>

TEST(EngineTest, GreetingReportsWiring) {
    orderbook::Engine engine;
    EXPECT_EQ(engine.greeting(), "matching-engine: build wiring OK");
}
