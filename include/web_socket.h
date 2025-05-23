#pragma once

#include "market_data_source.h"
#include <../libs/ixwebsocket/IXWebSocket.h>

class WebSocket : public MarketDataSource {
public:
    WebSocket(const std::string& url);
    ~WebSocket();

    // Disable copy semantics
    WebSocket(const WebSocket&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;

    // Disable move semantics
    WebSocket(WebSocket&&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

    void connect() override;
    void close() override;
    void send(const std::string& message) override;
    void recv(std::vector<std::string>& messages) override;
    bool is_connected() const override { return is_connected_; }
    void set_ping_options(int interval, const std::string& message); 

private:
    ix::WebSocket ws_;
    std::vector<std::string> received_messages_;
    std::mutex receive_mutex_;
    bool is_connected_ = false;
    std::string url_;
};