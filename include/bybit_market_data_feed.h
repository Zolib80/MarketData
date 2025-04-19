#pragma once

#include "event_loop.h"
#include "instrument.h"
#include "market_data_source.h"
#include "message_recorder.h"
#include "order_book.h"

class BybitMarketDataFeed {
public:
    BybitMarketDataFeed(
        EventLoop& event_loop,
        MarketDataSource* data_source,
        std::vector<const Instrument*>&& instruments_to_subscribe,
        MessageRecorder* recorder = nullptr);
    ~BybitMarketDataFeed();

    // Disable copy semantics
    BybitMarketDataFeed(const BybitMarketDataFeed&) = delete;
    BybitMarketDataFeed& operator=(const BybitMarketDataFeed&) = delete;

    // Disable move semantics
    BybitMarketDataFeed(BybitMarketDataFeed&&) = delete;
    BybitMarketDataFeed& operator=(BybitMarketDataFeed&&) = delete;

    void run();
    inline static const std::string BASE_URL = "wss://stream-testnet.bybit.com/v5/public/spot";

private:
    EventLoop& event_loop_;
    std::map<const Instrument*, OrderBook> order_books_;
    std::map<std::string, const Instrument*> instrument_map_;
    std::vector<std::string> received_messages_buffer_;
    MarketDataSource* data_source_;
    std::vector<const Instrument*> instruments_to_subscribe_;
    MessageRecorder* message_recorder_ = nullptr;
    bool is_connected_ = false;

    inline static const std::string PING_MESSAGE = R"({"op": "ping"})";
    static const int PING_INTERVAL_SECS = 20;

    void apply_snapshot(OrderBook& order_book, const nlohmann::json& snapshotData);
    void apply_delta(OrderBook& order_book, const nlohmann::json& deltaData);
};
