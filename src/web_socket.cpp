#include "web_socket.h"
#include <iostream>
#include <mutex>

WebSocket::WebSocket(const std::string& url) : url_(url), is_connected_(false) {}

WebSocket::~WebSocket() {
    close();
}

bool WebSocket::connect() {
    ws_.setUrl(url_);
    ws_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& message) {
        if (message->type == ix::WebSocketMessageType::Message) {
            std::lock_guard<std::mutex> lock(receive_mutex_);
            received_messages_.push_back(message->str);
        } else if (message->type == ix::WebSocketMessageType::Open) {
            std::cout << "WebSocket connected.\n";
            is_connected_ = true;
        } else if (message->type == ix::WebSocketMessageType::Close) {
            std::cout << "WebSocket closed.\n";
            std::cout << "  Code: " << message->closeInfo.code << '\n';
            std::cout << "  Reason: " << message->closeInfo.reason << '\n';
            std::cout << "  Initiated remotely: " << message->closeInfo.remote << '\n';
            is_connected_ = false;
        } else if (message->type == ix::WebSocketMessageType::Error) {
            std::cerr << "WebSocket Error:\n";
            std::cerr << "  Retries: " << message->errorInfo.retries << '\n';
            std::cerr << "  Wait Time: " << message->errorInfo.wait_time << '\n';
            std::cerr << "  HTTP Status: " << message->errorInfo.http_status << '\n';
            std::cerr << "  Reason: " << message->errorInfo.reason << '\n';
            std::cerr << "  Decompression Error: " << message->errorInfo.decompressionError << '\n';
            is_connected_ = false;
        } else if (message->type == ix::WebSocketMessageType::Pong) {
            std::cout << "Received Pong: " << message->str << '\n';
            is_connected_ = true;
        }
    });
    ws_.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return is_connected_;
}

void WebSocket::close() {
    ws_.stop();
    is_connected_ = false;
}

void WebSocket::send(const std::string& message) {
    if (is_connected_) {
        ws_.sendText(message);
    } else {
        std::cerr << "Cannot send: not connected.\n";
    }
}

void WebSocket::recv(std::vector<std::string>& messages) {
    std::unique_lock<std::mutex> lock(receive_mutex_);
    std::swap(messages, received_messages_);
    received_messages_.clear();
}

void WebSocket::set_ping_options(int ping_interval_secs, const std::string& ping_message) {
    ws_.setPingInterval(ping_interval_secs);
    ws_.setPingMessage(ping_message, ix::SendMessageKind::Ping);
}