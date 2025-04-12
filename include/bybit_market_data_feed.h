#pragma once

#include "event_loop.h"
#include "order_book.h"
#include "web_socket.h"

class BybitMarketDataFeed {
public:
    BybitMarketDataFeed(EventLoop& event_loop);
    void run();

private:
    EventLoop& event_loop_;
    OrderBook order_book_;
    bool snapshot_received_;
    std::vector<std::string> received_messages_buffer_;
    WebSocket ws;
};
