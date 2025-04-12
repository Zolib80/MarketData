#pragma once
#include <../libs/json/json.hpp> 
#include <map>
#include <iostream>
#include <compare>

struct price_t {
    double value = 0.0; // Default initialization
    explicit price_t(double v) : value(v) {}
    price_t() = default; // Default constructor
    explicit operator double() const { return value; } 

    // Use spaceship operator for comparisons
    auto operator<=> (const price_t& other) const = default;
};

struct quantity_t {
    double value = 0.0; // Default initialization
    explicit quantity_t(double v) : value(v) {}
    quantity_t() = default; // Default constructor
    explicit operator double() const { return value; } 

    // Use spaceship operator for comparisons
    auto operator<=> (const quantity_t& other) const = default;
};

class OrderBook {
    public:
        OrderBook() = default;

        void apply_snapshot(const nlohmann::json& snapshotData);
        void apply_delta(const nlohmann::json& deltaData);
    
        void print_order_book() const;
    
        std::map<price_t, quantity_t, std::greater<price_t>> bids_;
        std::map<price_t, quantity_t> asks_;
};