#include <iostream>

#include "orderbook/orderbook.hpp"

int main() {
    orderbook::Engine engine;
    std::cout << engine.greeting() << '\n';
    return 0;
}
