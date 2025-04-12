#pragma once

#include "event_loop.h"
#include "instrument.h"
#include "order_book.h"
#include "web_socket.h"

class BybitMarketDataFeed {
public:
    BybitMarketDataFeed(EventLoop& event_loop, std::vector<const Instrument*>&& instruments_to_subscribe);
    void run();

private:
    EventLoop& event_loop_;
    std::map<const Instrument*, OrderBook> order_books_;
    std::map<std::string, const Instrument*> instrument_map_;
    std::vector<std::string> received_messages_buffer_;
    WebSocket ws_;
    std::vector<const Instrument*> instruments_to_subscribe_;

    inline static const std::string BASE_URL = "wss://stream-testnet.bybit.com/v5/public/spot";
    // inline static const std::string SUBSCRIBE_MESSAGE = R"({"req_id": "test","op": "subscribe","args": ["orderbook.50.BTCUSDT"]})";
    inline static const std::string PING_MESSAGE = R"({"op": "ping"})";
    static const int PING_INTERVAL_SECS = 20;

    void apply_snapshot(OrderBook& order_book, const nlohmann::json& snapshotData);
    void apply_delta(OrderBook& order_book, const nlohmann::json& deltaData);
};
