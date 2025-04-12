#include "bybit_market_data_feed.h"
#include "event_loop.h"
#include "order_book.h"
#include "web_socket.h"

#include <../libs/ixwebsocket/IXWebSocket.h>
#include <../libs/json/json.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <functional>


BybitMarketDataFeed::BybitMarketDataFeed(EventLoop& event_loop)
    : event_loop_(event_loop)
    , snapshot_received_(false)
    , order_book_()
    , ws(WebSocket::URL) {}

void BybitMarketDataFeed::run() {
    if (!ws.is_connected()) {
        ws.connect();
        std::cout << "Connected to Bybit.\n";
        return;
    }
    ws.recv(received_messages_buffer_);
    for (const auto& rawMessage : received_messages_buffer_) {
        nlohmann::json jsonMessage = nlohmann::json::parse(rawMessage);
        if (jsonMessage.contains("topic") && jsonMessage["topic"] == "orderbook.50.BTCUSDT") {
            if (jsonMessage["type"] == "snapshot") {
                order_book_.apply_snapshot(jsonMessage["data"]);
                snapshot_received_ = true;
            } else if (jsonMessage["type"] == "delta" && snapshot_received_) {
                order_book_.apply_delta(jsonMessage["data"]);
            } else if (jsonMessage["type"] == "delta" && !snapshot_received_) {
                std::cerr << "Received delta before snapshot. Ignoring.\n";
            } else if (jsonMessage.contains("success") && jsonMessage["success"] == true && jsonMessage["op"] == "subscribe") {
                std::cout << "Subscription successful.\n";
            } else if (jsonMessage.contains("op") && jsonMessage["op"] == "ping") {
                // Pong message is handled by the web_socket class
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
