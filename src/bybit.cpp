#include <iostream>
#include "web_socket.hpp"

int main() {
    std::cout << "Application started" << std::endl;
    int reconnectDelayMS = 5000;
    int reconnectAttemptCount = 0;
    bool shouldReconnect = true;
    
    while (shouldReconnect) {
        web_socket ws(web_socket::URL);
        std::cout << "WebSocket client created." << std::endl;

        if (ws.connect()) {
            std::cout << "Connected to WebSocket." << std::endl;
            reconnectAttemptCount = 0;
            try {
                while (true) {
                    std::string message = ws.recv();
                    std::cout << "Received: " << message << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error during communication: " << e.what() << std::endl;
                std::cout << "Attempting to reconnect..." << std::endl;
            }
        } else {
            std::cerr << "Failed to connect to WebSocket." << std::endl;
            reconnectAttemptCount++;
            if (reconnectAttemptCount >= ws.maxReconnectAttempts) {
                std::cerr << "Maximum reconnection attempts reached. Stopping." << std::endl;
                shouldReconnect = false;
            }
        }

        std::cout << "Closing WebSocket client." << std::endl;
        ws.close();

        if (shouldReconnect) {
            std::cout << "Waiting " << reconnectDelayMS << " ms before reconnecting..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(reconnectDelayMS));
            reconnectDelayMS *= 2;
            if (reconnectDelayMS > 60000) {
                reconnectDelayMS = 60000;
            }
        }

        std::cout << "Reconnection attempts stopped." << std::endl;
    }
    return 0;
}