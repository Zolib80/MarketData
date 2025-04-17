#pragma once

#include "market_data_constants.h"
#include "market_data_source.h"
#include "market_data_time.h"

#include <condition_variable>
#include <deque>
#include <fstream>

class FilePlayback : public MarketDataSource {
public:
    FilePlayback(const std::string& filename);
    ~FilePlayback() override;

    bool connect() override;
    void close() override;
    void send(const std::string& message) override;
    void recv(std::vector<std::string>& messages) override;
    bool is_connected() const override { return is_connected_; }

private:
    std::ifstream infile_;
    std::string filename_;
    bool is_connected_ = false;
    timestamp first_message_time_;
    timestamp previous_message_time_;
    std::deque<std::string> message_buffer_;
    bool is_running_ = false;
};