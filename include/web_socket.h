#pragma once

#include "queue.h"
#include <../libs/ixwebsocket/IXWebSocket.h>
#include <string>
#include <mutex>

class WebSocket {
public:
    WebSocket(const std::string& url);
    ~WebSocket();

    // Disable copy semantics
    WebSocket(const WebSocket&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;

    // Disable move semantics
    WebSocket(WebSocket&&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

    bool is_connected() const { return is_connected_; }
    bool connect();
    void close();
    void send(const std::string& message);
    void recv(std::vector<std::string>& messages);
    void set_ping_options(int interval, const std::string& message); 

private:
    ix::WebSocket ws_;
    std::vector<std::string> received_messages_;
    std::mutex receive_mutex_;
    bool is_connected_ = false;
    std::string url_;
};