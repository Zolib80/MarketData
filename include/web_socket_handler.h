#pragma once
#include <../libs/ixwebsocket/IXWebSocket.h>
#include <../libs/json/json.hpp>

#include "order_book.h"
#include "event_loop.h"

class BybitMarketDataFeed {
public:
    BybitMarketDataFeed(EventLoop& event_loop);
    void run();

private:
    EventLoop& event_loop_;
    OrderBook order_book_;
    bool snapshot_received_;
    std::vector<std::string> received_messages_buffer_;
    int reconnect_delay_ms_;
    int reconnect_attempt_count_;
    bool should_reconnect_;
};