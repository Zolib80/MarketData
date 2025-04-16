#pragma once

#include "market_data_time.h"

#include <fstream>
#include <vector>

constexpr uint32_t MAGIC_NUMBER = 0x7A626D64;
constexpr uint8_t VERSION_NUMBER = 1;

enum class MessageType : uint8_t {
    Incoming = 0,
    Outgoing = 1,
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