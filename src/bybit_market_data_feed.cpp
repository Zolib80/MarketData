#include "bybit_market_data_feed.h"
#include "event_loop.h"
#include "order_book.h"
#include "spsc_byte_ring_buffer.h" 
#include "web_socket.h"

#include <../libs/ixwebsocket/IXWebSocket.h>
#include <../libs/json/json.hpp>

BybitMarketDataFeed::BybitMarketDataFeed(
    EventLoop& event_loop,
    MarketDataSource* data_source,
    std::vector<const Instrument*>&& instruments_to_subscribe,
    SpscByteRingBuffer* ring_buffer)
    : event_loop_(event_loop)
    , order_books_()
    , data_source_(data_source)
    , instruments_to_subscribe_(std::move(instruments_to_subscribe))
    , ring_buffer_(ring_buffer)
{}

BybitMarketDataFeed::~BybitMarketDataFeed() { }

void BybitMarketDataFeed::run() {
    if (!data_source_) {
        std::cerr << "Error: No data source set.\n";
        return;
    }

    bool data_source_connected = data_source_->is_connected();

    if (data_source_connected && !is_connected_) {
        event_loop_.remove_event(this);
        is_connected_ = true;
        record_message(MessageType::Connect, "");
        std::cout << "Connected to Bybit.\n";

        nlohmann::json subscribe_message_json;
        subscribe_message_json["op"] = "subscribe";
        subscribe_message_json["args"] = nlohmann::json::array();
        for (const Instrument* instrument : instruments_to_subscribe_) {
            subscribe_message_json["args"].push_back("orderbook.50." + instrument->name_);
            instrument_map_[instrument->name_] = instrument;
        }
        std::string subscribe_message = subscribe_message_json.dump();
        std::cout << "Subscribing to: " << subscribe_message << '\n';
        record_message(MessageType::Outgoing, subscribe_message);
        data_source_->send(subscribe_message);
    }    

    data_source_->recv(received_messages_buffer_);
    for (const auto& rawMessage : received_messages_buffer_) {
        record_message(MessageType::Incoming, rawMessage);
        nlohmann::json jsonMessage = nlohmann::json::parse(rawMessage);
        if (jsonMessage.contains("topic")) {
            std::string topic = jsonMessage["topic"].get<std::string>();
            if (topic.rfind("orderbook.50.", 0) == 0) {
                std::string instrument_name = topic.substr(std::string("orderbook.50.").length());
                const Instrument* instrument = instrument_map_[instrument_name];
                
                if (instrument == nullptr) {
                    std::cerr << "Unknown instrument: " << instrument_name << '\n';
                    continue;
                }
                
                if (jsonMessage["type"] == "snapshot") {
                    OrderBook& current_order_book = order_books_.emplace(instrument, OrderBook(instrument)).first->second;
                    apply_snapshot(current_order_book, jsonMessage["data"]);
                } else if (jsonMessage["type"] == "delta") {
                    auto it = order_books_.find(instrument);

                    if (it != order_books_.end()) {
                        OrderBook& current_order_book = it->second;
                        apply_delta(current_order_book, jsonMessage["data"]);
                    } else {
                        std::cerr << "Received delta before snapshot for " << instrument_name << ". Ignoring.\n";
                    }

                } else if (jsonMessage.contains("success") && jsonMessage["success"] == true && jsonMessage["op"] == "subscribe") {
                    std::cout << "Subscription successful for: " << rawMessage << '\n';
                } else if (jsonMessage.contains("op") && jsonMessage["op"] == "ping") {
                    // Pong handled by WebSocket class
                } else {
                    std::cout << "Received unknown message: " << rawMessage << '\n';
                }
            } else if (jsonMessage.contains("success")) {
                std::cout << "General success message: " << rawMessage << '\n';
            } else {
                std::cout << "Received other message: " << rawMessage << '\n';
            }
        }
    }

    if (received_messages_buffer_.empty() && !data_source_connected && is_connected_) {
        is_connected_ = false;
        record_message(MessageType::Disconnect, "");
        event_loop_.schedule_repeating_event(this, 0_us, 5_s, [this]() {
            data_source_->connect();
        });
    }    
}

void BybitMarketDataFeed::apply_snapshot(OrderBook& order_book, const nlohmann::json& snapshot_data) {
    if (snapshot_data.contains("b") && snapshot_data["b"].is_array()) {
        for (const auto& bid : snapshot_data["b"]) {
            if (bid.is_array() && bid.size() == 2) {
                price_t price{std::stod(bid[0].get<std::string>())};
                quantity_t quantity{std::stod(bid[1].get<std::string>())};
                order_book.bids_[price] = quantity;
            }
        }
    }

    if (snapshot_data.contains("a") && snapshot_data["a"].is_array()) {
        for (const auto& ask : snapshot_data["a"]) {
            if (ask.is_array() && ask.size() == 2) {
                price_t price{std::stod(ask[0].get<std::string>())};
                quantity_t quantity{std::stod(ask[1].get<std::string>())};
                order_book.asks_[price] = quantity;
            }
        }
    }
    std::cout << "Snapshot applied for " << order_book.instrument_->name_ << ".\n";
}

void BybitMarketDataFeed::apply_delta(OrderBook& order_book, const nlohmann::json& delta_data) {
    if (delta_data.contains("b") && delta_data["b"].is_array()) {
        for (const auto& bid_delta : delta_data["b"]) {
            if (bid_delta.is_array() && bid_delta.size() == 2) {
                price_t price{std::stod(bid_delta[0].get<std::string>())};
                quantity_t quantity{std::stod(bid_delta[1].get<std::string>())};
                if (quantity.value > 0) {
                    order_book.bids_[price] = quantity;
                } else {
                    order_book.bids_.erase(price);
                }
            }
        }
    }

    if (delta_data.contains("a") && delta_data["a"].is_array()) {
        for (const auto& ask_delta : delta_data["a"]) {
            if (ask_delta.is_array() && ask_delta.size() == 2) {
                price_t price{std::stod(ask_delta[0].get<std::string>())};
                quantity_t quantity{std::stod(ask_delta[1].get<std::string>())};
                if (quantity.value > 0) {
                    order_book.asks_[price] = quantity;
                } else {
                    order_book.asks_.erase(price);
                }
            }
        }
    }
    std::cout << "Delta applied for " << order_book.instrument_->name_ << ".\n";
}

void BybitMarketDataFeed::record_message(MessageType type, const std::string& message) {
    if (ring_buffer_) {
        ring_buffer_->write(event_loop_.get_current_time(), type, message);
    }
}