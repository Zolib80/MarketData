#include "message_recorder.h"

#include <iostream>

MessageRecorder::MessageRecorder(const std::string& filename) : outfile_(filename, std::ios::binary) {
    if (!outfile_.is_open()) {
        std::cerr << "Error opening file for recording: " << filename << '\n';
    } else {
        outfile_.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
        outfile_.write(reinterpret_cast<const char*>(&VERSION_NUMBER), sizeof(VERSION_NUMBER));
        timestamp null_time{};
        outfile_.write(reinterpret_cast<const char*>(&null_time), sizeof(null_time));
        outfile_.write(reinterpret_cast<const char*>(&null_time), sizeof(null_time));
    }
}

MessageRecorder::~MessageRecorder() {
    if (outfile_.is_open()) {
        outfile_.close();
    }
}

void MessageRecorder::record_message(const timestamp& current_time, MessageType type, const std::string& message) {
    if (!outfile_.is_open()) return;
    if (is_first_message_) {
        outfile_.seekp(sizeof(MAGIC_NUMBER) + sizeof(VERSION_NUMBER));
        outfile_.write(reinterpret_cast<const char*>(&current_time), sizeof(current_time));
        outfile_.seekp(0, std::ios::end);
        is_first_message_ = false;
    }

    outfile_.write(reinterpret_cast<const char*>(&current_time), sizeof(current_time));
    outfile_.write(reinterpret_cast<const char*>(&type), sizeof(type));
    uint32_t message_length = static_cast<uint32_t>(message.length());
    outfile_.write(reinterpret_cast<const char*>(&message_length), sizeof(message_length));
    outfile_.write(message.data(), message_length);
}

void MessageRecorder::finalize_recording(const timestamp& last_time) {
    if (!outfile_.is_open()) return;
    outfile_.seekp(sizeof(MAGIC_NUMBER) + sizeof(VERSION_NUMBER) + sizeof(timestamp));
    outfile_.write(reinterpret_cast<const char*>(&last_time), sizeof(last_time));
}