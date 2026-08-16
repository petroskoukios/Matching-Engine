#include <iostream>

#include "orderbook/orderbook.hpp"

int main() {
    orderbook::OrderBook book;
    std::vector<orderbook::Trade> trades;
    orderbook::Order order1{.id = 1,
                            .price = 100,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};
    orderbook::Order order2{.id = 2,
                            .price = 100,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};
    orderbook::Order order3{.id = 3,
                            .price = 101,
                            .quantity = 5,
                            .side = orderbook::Side::Buy,
                            .type = orderbook::OrderType::Limit};
    orderbook::Order order4{.id = 4,
                            .price = 102,
                            .quantity = 5,
                            .side = orderbook::Side::Sell,
                            .type = orderbook::OrderType::Limit};
    orderbook::Order order5{.id = 5,
                            .price = 103,
                            .quantity = 5,
                            .side = orderbook::Side::Sell,
                            .type = orderbook::OrderType::Limit};
    orderbook::Order order6{.id = 6,
                            .price = 103,
                            .quantity = 5,
                            .side = orderbook::Side::Sell,
                            .type = orderbook::OrderType::Limit};

    std::cout << book.submit(order1, trades) << "\n";
    std::cout << book.submit(order2, trades) << "\n";
    std::cout << book.submit(order3, trades) << "\n";
    std::cout << book.submit(order4, trades) << "\n";
    std::cout << book.submit(order5, trades) << "\n";
    std::cout << book.submit(order6, trades) << "\n";
    std::cout << book.submit(order1, trades) << "\n";

    if (auto result = book.best_bid()) {
        std::cout << "The best bid is: " << result.value() << "\n";
    }
    if (auto result = book.best_ask()) {
        std::cout << "The best ask is: " << result.value() << "\n";
    }

    std::cout << book.quantity_at(orderbook::Side::Buy, 100) << "\n";

    std::cout << book.cancel(3) << "\n";

    if (auto result = book.best_bid()) std::cout << "Best bid:" << result.value() << "\n";

    return 0;
}