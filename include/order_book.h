#include <../libs/json/json.hpp> 

struct price_t {
    double value;
    explicit price_t(double v) : value(v) {}
    explicit operator double() const { return value; } 
};

struct quantity_t {
    double value;
    explicit quantity_t(double v) : value(v) {}
    explicit operator double() const { return value; } 
};

class OrderBook {
    public:
        OrderBook() = default;
    
        void apply_snapshot(const nlohmann::json& snapshotData);
        void apply_delta(const nlohmann::json& deltaData);
    
        const std::map<price_t, quantity_t, std::greater<double>>& get_bids() const { return bids_; }
        const std::map<price_t, quantity_t>& get_asks() const { return asks_; }
    
        void print_order_book() const;
    
    private:
        std::map<price_t, quantity_t, std::greater<double>> bids_;
        std::map<price_t, quantity_t> asks_;
};