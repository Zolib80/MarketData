#pragma once

#include <instrument.h> 

#include <../libs/json/json.hpp> 
#include <map>
#include <iostream>
#include <compare>

struct price_t {
    double value = 0.0;
    explicit price_t(double v) : value(v) {}
    price_t() = default;
    explicit operator double() const { return value; }

    std::partial_ordering operator<=> (const price_t& other) const {
        if (std::isnan(value) || std::isnan(other.value)) return std::partial_ordering::unordered;
        if (value < other.value) return std::partial_ordering::less;
        if (value > other.value) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    }
};

struct quantity_t {
    double value = 0.0;
    explicit quantity_t(double v) : value(v) {}
    quantity_t() = default;
    explicit operator double() const { return value; }

    std::partial_ordering operator<=> (const quantity_t& other) const {
        if (std::isnan(value) || std::isnan(other.value)) return std::partial_ordering::unordered;
        if (value < other.value) return std::partial_ordering::less;
        if (value > other.value) return std::partial_ordering::greater;
        return std::partial_ordering::equivalent;
    }
};

class OrderBook {
    public:
        OrderBook(const Instrument* instrument) : instrument_(instrument) {};
    
        void print_order_book() const;
    
        std::map<price_t, quantity_t, std::greater<price_t>> bids_;
        std::map<price_t, quantity_t> asks_;
        const Instrument* instrument_;
};