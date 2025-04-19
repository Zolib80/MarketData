#pragma once

#include "event_loop.h"
#include "market_data_constants.h"
#include "market_data_source.h"
#include "market_data_time.h"
#include "message_recorder.h"

#include <fstream>

class FilePlayback : public MarketDataSource {
public:
    FilePlayback(EventLoop& event_loop, const std::string& filename);
    ~FilePlayback() override;

    void connect() override;
    void close() override;
    void send(const std::string& message) override;
    void recv(std::vector<std::string>& messages) override;
    bool is_connected() const override { return is_connected_; }
    
    private:
    struct Message {
        MessageType type;
        std::string content;
    };
    
    EventLoop& event_loop_;
    std::string filename_;
    std::ifstream infile_;
    timestamp first_message_time_;
    timestamp next_message_time_;
    std::vector<Message> upcoming_messages_;
    bool is_connected_ = false;
    
    void preload_messages();
    void stop();
};