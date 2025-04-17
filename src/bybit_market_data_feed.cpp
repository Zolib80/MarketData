#include "bybit_market_data_feed.h"
#include "event_loop.h"
#include "order_book.h"
#include "web_socket.h"

#include <../libs/ixwebsocket/IXWebSocket.h>
#include <../libs/json/json.hpp>

BybitMarketDataFeed::BybitMarketDataFeed(
    EventLoop& event_loop,
    MarketDataSource* data_source,
    std::vector<const Instrument*>&& instruments_to_subscribe,
    MessageRecorder* recorder)
    : event_loop_(event_loop)
    , order_books_()
    , data_source_(data_source)
    , instruments_to_subscribe_(std::move(instruments_to_subscribe))
    , message_recorder_(recorder) {}

BybitMarketDataFeed::~BybitMarketDataFeed()
{
    if (message_recorder_) {
        message_recorder_->finalize_recording(event_loop_.get_current_time());
    }
}

void BybitMarketDataFeed::run() {
    if (!data_source_) {
        std::cerr << "Error: No data source set.\n";
        return;
    }

    if (is_playback_finished_) {
        std::cout << "Playback already finished. Finishing event loop.\n";
        event_loop_.stop();
        return;
    }

    if (!data_source_->is_connected()) {
        if (data_source_->connect()) {
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
            if (message_recorder_) {
                message_recorder_->record_message(event_loop_.get_current_time(), MessageType::Outgoing, subscribe_message);
            }
            data_source_->send(subscribe_message);
            
            return;
        } else {
            std::cerr << "Failed to connect to Bybit.\n";
            return;
        }
    }

    data_source_->recv(received_messages_buffer_);
    if (received_messages_buffer_.empty() && !data_source_->is_connected()) {
        is_playback_finished_ = true;
        std::cout << "Playback completed.\n";
        return;
    }
    for (const auto& rawMessage : received_messages_buffer_) {
        if (message_recorder_) {
            message_recorder_->record_message(event_loop_.get_current_time(), MessageType::Incoming, rawMessage);
        }
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