#include "spsc_byte_ring_buffer.h"
#include "message_recorder.h"

#include <fstream>
#include <iostream>

MessageRecorder::MessageRecorder(const std::string& filename, SpscByteRingBuffer& ring_buffer)
    : filename_(filename)
    , ring_buffer_(ring_buffer)
    , recorder_thread_(&MessageRecorder::recorder_thread_func, this) 
{ }

MessageRecorder::~MessageRecorder() {
    stop_flag_ = true;
    if (recorder_thread_.joinable()) {
        recorder_thread_.join();
    }
}

void MessageRecorder::recorder_thread_func() {
    std::ofstream outfile_(filename_, std::ios::binary);
    if (!outfile_.is_open()) {
        std::cerr << "Error opening file for recording: " << filename_ << '\n';
        return;
    }

    outfile_.write(reinterpret_cast<const char*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
    outfile_.write(reinterpret_cast<const char*>(&VERSION_NUMBER), sizeof(VERSION_NUMBER));
    timestamp first_message_ts{};
    timestamp last_message_ts{};
    outfile_.write(reinterpret_cast<const char*>(&first_message_ts), sizeof(first_message_ts));
    outfile_.write(reinterpret_cast<const char*>(&last_message_ts), sizeof(last_message_ts));
    bool first_message = true;

    auto processor = [&](timestamp ts, MessageType type, std::string_view message) {
        if (first_message) {
            first_message_ts = ts;
            outfile_.seekp(sizeof(MAGIC_NUMBER) + sizeof(VERSION_NUMBER));
            outfile_.write(reinterpret_cast<const char*>(&first_message_ts), sizeof(first_message_ts));
            outfile_.seekp(0, std::ios::end);
            first_message = false;
        }
        record_message(outfile_, ts, type, message);
        last_message_ts = ts;
    };

    while (!stop_flag_) {
        if (!ring_buffer_.try_read(processor)) {
            std::this_thread::yield();
        }
    }

    if (!first_message) {
        finalize_recording(outfile_, last_message_ts);
    }
    outfile_.close();
    std::cout << "Recorder thread finished.\n";
}

void MessageRecorder::record_message(std::ofstream& outfile, const timestamp& ts, MessageType type, std::string_view message) {
    outfile.write(reinterpret_cast<const char*>(&ts), sizeof(ts));
    outfile.write(reinterpret_cast<const char*>(&type), sizeof(type));
    uint32_t message_length = static_cast<uint32_t>(message.length());
    outfile.write(reinterpret_cast<const char*>(&message_length), sizeof(message_length));
    outfile.write(message.data(), message_length);
}

void MessageRecorder::finalize_recording(std::ofstream& outfile, const timestamp& last_time) {
    if (!outfile.is_open()) return;
    outfile.seekp(sizeof(MAGIC_NUMBER) + sizeof(VERSION_NUMBER) + sizeof(timestamp));
    outfile.write(reinterpret_cast<const char*>(&last_time), sizeof(last_time));
}