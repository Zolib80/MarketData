#pragma once

#include "spsc_byte_ring_buffer.h"
#include "market_data_constants.h"
#include "market_data_time.h"
#include "message_type.h"

#include <atomic>
#include <fstream>
#include <functional>
#include <string>
#include <thread>


class MessageRecorder {
public:
    MessageRecorder(const std::string& filename, SpscByteRingBuffer& ring_buffer);
    ~MessageRecorder();

    // Disable copy semantics
    MessageRecorder(const MessageRecorder&) = delete;
    MessageRecorder& operator=(const MessageRecorder&) = delete;

    // Disable move semantics
    MessageRecorder(MessageRecorder&&) = delete; 
    MessageRecorder& operator=(MessageRecorder&&) = delete;

    
private:
    std::string filename_;
    SpscByteRingBuffer& ring_buffer_;
    std::thread recorder_thread_;
    std::atomic<bool> stop_flag_{false};
    size_t read_offset_{0};

    void recorder_thread_func();
    void set_record_header(std::ofstream& outfile);
    void start_recording(std::ofstream& outfile, const QueueHeader &header);
    void record_message(std::ofstream& outfile, const QueueHeader& header, std::string_view message);
    void finalize_recording(std::ofstream& outfile, const timestamp& last_time);
};