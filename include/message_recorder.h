#pragma once

#include "market_data_constants.h"
#include "market_data_time.h"

#include <fstream>
#include <vector>

enum class MessageType : uint8_t {
    Connect = 0,
    Disconnect,
    Incoming,
    Outgoing,
};

class MessageRecorder {
public:
    MessageRecorder(const std::string& filename);
    ~MessageRecorder();

    // Disable copy semantics
    MessageRecorder(const MessageRecorder&) = delete;
    MessageRecorder& operator=(const MessageRecorder&) = delete;

    // Disable move semantics
    MessageRecorder(MessageRecorder&&) = delete;
    MessageRecorder& operator=(MessageRecorder&&) = delete;

    void record_message(const timestamp& current_time, MessageType message_type, const std::string& message);
    void finalize_recording(const timestamp& last_time);

private:
    std::ofstream outfile_;

    bool is_first_message_ = true;
};