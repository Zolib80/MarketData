#include "web_socket.hpp"
#include <iostream>
#include <mutex>
#include <condition_variable>

web_socket::web_socket(const std::string& url) : isConnected(false) {}

web_socket::~web_socket() {
    close();
}

bool web_socket::connect() {
    ws.setUrl(URL);
    ws.setOnMessageCallback([this](const ix::WebSocketMessagePtr& message) {
        if (message->type == ix::WebSocketMessageType::Message) {
            {
                std::lock_guard<std::mutex> lock(receiveMutex);
                receivedMessage = message->str;
                isMessageReceived = true;
            }
            receiveConditionVariable.notify_one();
        } else if (message->type == ix::WebSocketMessageType::Open) {
            std::cout << "WebSocket connected." << std::endl;
            isConnected = true;
            send(SUBSCRIBE_MESSAGE);
            setPingOptions();
        } else if (message->type == ix::WebSocketMessageType::Close) {
            std::cout << "WebSocket closed." << std::endl;
            std::cout << "  Code: " << message->closeInfo.code << std::endl;
            std::cout << "  Reason: " << message->closeInfo.reason << std::endl;
            std::cout << "  Initiated remotely: " << message->closeInfo.remote << std::endl;
            isConnected = false;
        } else if (message->type == ix::WebSocketMessageType::Error) {
            std::cerr << "WebSocket Error:" << std::endl;
            std::cerr << "  Retries: " << message->errorInfo.retries << std::endl;
            std::cerr << "  Wait Time: " << message->errorInfo.wait_time << std::endl;
            std::cerr << "  HTTP Status: " << message->errorInfo.http_status << std::endl;
            std::cerr << "  Reason: " << message->errorInfo.reason << std::endl;
            std::cerr << "  Decompression Error: " << message->errorInfo.decompressionError << std::endl;
            isConnected = false;
        } else if (message->type == ix::WebSocketMessageType::Pong) {
            std::cout << "Received Pong: " << message->str << std::endl;
            isConnected = true;
        }
    });
    ws.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return isConnected;
}

void web_socket::close() {
    ws.stop();
    isConnected = false;
}

void web_socket::send(const std::string& message) {
    if (isConnected) {
        ws.sendText(message);
    } else {
        std::cerr << "Cannot send: not connected." << std::endl;
    }
}

std::string web_socket::recv() {
    std::unique_lock<std::mutex> lock(receiveMutex);
    receiveConditionVariable.wait(lock, [this]{ return isMessageReceived; });
    isMessageReceived = false;
    return receivedMessage;
}

void web_socket::setPingOptions() {
    std::lock_guard<std::mutex> lock(configMutex);
    ws.setPingInterval(pingIntervalSecs);
    ws.setPingMessage(PING_MESSAGE, ix::SendMessageKind::Ping);
}