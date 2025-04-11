#include "order_book.h"
#include <iostream>

void OrderBook::apply_snapshot(const nlohmann::json& snapshot_data) {
    bids_.clear();
    asks_.clear();

    if (snapshot_data.contains("b") && snapshot_data["b"].is_array()) {
        for (const auto& bid : snapshot_data["b"]) {
            if (bid.is_array() && bid.size() == 2) {
                try {
                    price_t price(std::stod(bid[0].get<std::string>()));
                    quantity_t quantity(std::stod(bid[1].get<std::string>()));
                    bids_[price] = quantity;
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing bid (snapshot): " << e.what() << '\n';
                }
            }
        }
    }

    if (snapshot_data.contains("a") && snapshot_data["a"].is_array()) {
        for (const auto& ask : snapshot_data["a"]) {
            if (ask.is_array() && ask.size() == 2) {
                try {
                    price_t price(std::stod(ask[0].get<std::string>()));
                    quantity_t quantity(std::stod(ask[1].get<std::string>()));
                    asks_[price] = quantity;
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing ask (snapshot): " << e.what() << '\n';
                }
            }
        }
    }
    std::cout << "Snapshot applied.\n";
    print_order_book();
}

void OrderBook::apply_delta(const nlohmann::json& delta_data) {
    if (delta_data.contains("b") && delta_data["b"].is_array()) {
        for (const auto& bid_delta : delta_data["b"]) {
            if (bid_delta.is_array() && bid_delta.size() == 2) {
                try {
                    price_t price(std::stod(bid_delta[0].get<std::string>()));
                    quantity_t quantity(std::stod(bid_delta[1].get<std::string>()));
                    if (quantity.value > 0) {
                        bids_[price] = quantity;
                    } else {
                        bids_.erase(price);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing bid delta: " << e.what() << '\n';
                }
            }
        }
    }

    if (delta_data.contains("a") && delta_data["a"].is_array()) {
        for (const auto& ask_delta : delta_data["a"]) {
            if (ask_delta.is_array() && ask_delta.size() == 2) {
                try {
                    price_t price(std::stod(ask_delta[0].get<std::string>()));
                    quantity_t quantity(std::stod(ask_delta[1].get<std::string>()));
                    if (quantity.value > 0) {
                        asks_[price] = quantity;
                    } else {
                        asks_.erase(price);
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error parsing ask delta: " << e.what() << '\n';
                }
            }
        }
    }
    std::cout << "Delta applied.\n";
    print_order_book();
}

void OrderBook::print_order_book() const {
    std::cout << "--- Order Book ---\n";
    std::cout << "Bids (price -> quantity):\n";
    for (const auto& bid : bids_) {
        std::cout << std::fixed << std::setprecision(8) << "  " << bid.first.value << " -> " << bid.second.value << '\n';
    }
    std::cout << "Asks (price -> quantity):\n";
    for (const auto& ask : asks_) {
        std::cout << std::fixed << std::setprecision(8) << "  " << ask.first.value << " -> " << ask.second.value << '\n';
    }
    std::cout << "--------------------\n\n";
}