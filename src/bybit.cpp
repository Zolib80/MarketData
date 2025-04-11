#include <iostream>
#include <../libs/json/json.hpp>
#include "web_socket.h"
#include "order_book.h" 

int main() {
    std::cout << "Application started\n";
    int reconnect_delay_ms = 5000;
    int reconnect_attempt_count = 0;
    bool should_reconnect = true;
    std::vector<std::string> received_messages_buffer;
    OrderBook order_book;
    bool snapshot_received = false;
    
    while (should_reconnect) {
        WebSocket ws(WebSocket::URL);
        std::cout << "WebSocket client created.\n";

        if (ws.connect()) {
            std::cout << "Connected to WebSocket.\n";
            reconnect_attempt_count = 0;
            try {
                while (true) {
                    bool isNewMessageReceived = ws.recv(received_messages_buffer);
                    if (isNewMessageReceived) {
                        for (const auto& rawMessage : received_messages_buffer) {
                            try {
                                nlohmann::json jsonMessage = nlohmann::json::parse(rawMessage);
                                if (jsonMessage.contains("topic") && jsonMessage["topic"] == "orderbook.50.BTCUSDT") {
                                    if (jsonMessage["type"] == "snapshot") {
                                        order_book.apply_snapshot(jsonMessage["data"]);
                                        snapshot_received = true;
                                    } else if (jsonMessage["type"] == "delta" && snapshot_received) {
                                        order_book.apply_delta(jsonMessage["data"]);
                                    } else if (jsonMessage["type"] == "delta" && !snapshot_received) {
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
                            } catch (const nlohmann::json::parse_error& e) {
                                std::cerr << "Error parsing JSON: " << e.what() << " - Raw message: " << rawMessage << '\n';
                            }
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error during communication: " << e.what() << '\n';
                std::cout << "Attempting to reconnect...\n";
            }
        } else {
            std::cerr << "Failed to connect to WebSocket.\n";
            reconnect_attempt_count++;
            if (reconnect_attempt_count >= ws.MAX_RECONNECT_ATTEMPTS) {
                std::cerr << "Maximum reconnection attempts reached. Stopping.\n";
                should_reconnect = false;
            }
        }

        std::cout << "Closing WebSocket client.\n";
        ws.close();

        if (should_reconnect) {
            std::cout << "Waiting " << reconnect_delay_ms << " ms before reconnecting...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(reconnect_delay_ms));
            reconnect_delay_ms *= 2;
            if (reconnect_delay_ms > 60000) {
                reconnect_delay_ms = 60000;
            }
        }

        std::cout << "Reconnection attempts stopped.\n";
    }
    return 0;
}