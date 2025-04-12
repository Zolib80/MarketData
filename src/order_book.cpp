#include "order_book.h"
#include <iostream>

void OrderBook::print_order_book() const {
    std::cout << "--- Order Book ---\n";
    
    auto print_orders = [&](const std::string& side, const auto& orders) {
        std::cout << side << " (price -> quantity):\n";
        for (const auto& order : orders) {
            std::cout << std::fixed << std::setprecision(8) << "  " << static_cast<double>(order.first) << " -> " << static_cast<double>(order.second) << '\n';
        }
    };

    print_orders("Bids", bids_);
    print_orders("Asks", asks_);

    std::cout << "--------------------\n\n";
}