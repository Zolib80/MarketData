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
    std::ofstream outfile(filename_, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for recording: " << filename_ << '\n';
        return;
    }

    set_record_header(outfile);
    bool first_message = true;
    timestamp last_message_ts{};

    while (!stop_flag_) {
        if (auto data = ring_buffer_.read_next(read_offset_)) {
            const auto& [header, message] = data.value();
            if (first_message) {
                start_recording(outfile, header);
                first_message = false;
            }
            record_message(outfile, header, message);
            last_message_ts = header.timestamp;
        } else {
            std::this_thread::yield(); // Várakozás, ha nincs mit olvasni
        }
    }

    if (!first_message) {
        finalize_recording(outfile, last_message_ts);
    }
    outfile.close();
    std::cout << "Recorder thread finished.\n";
}

void MessageRecorder::set_record_header(std::ofstream &outfile) {
    timestamp null_time{};
    outfile.write(reinterpret_cast<const char *>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));
    outfile.write(reinterpret_cast<const char *>(&VERSION_NUMBER), sizeof(VERSION_NUMBER));
    outfile.write(reinterpret_cast<const char *>(&null_time), sizeof(null_time));
    outfile.write(reinterpret_cast<const char *>(&null_time), sizeof(null_time));
}

void MessageRecorder::start_recording(std::ofstream &outfile, const QueueHeader &header) {
    outfile.seekp(sizeof(MAGIC_NUMBER) + sizeof(VERSION_NUMBER));
    outfile.write(reinterpret_cast<const char *>(&header.timestamp), sizeof(header.timestamp));
    outfile.seekp(0, std::ios::end);
}

void MessageRecorder::record_message(std::ofstream& outfile, const QueueHeader& header, std::string_view message) {
    outfile.write(reinterpret_cast<const char*>(&header.timestamp), sizeof(header.timestamp));
    outfile.write(reinterpret_cast<const char*>(&header.message_type), sizeof(header.message_type));
    outfile.write(reinterpret_cast<const char*>(&header.message_length), sizeof(header.message_length));
    outfile.write(message.data(), header.message_length);
}

void MessageRecorder::finalize_recording(std::ofstream& outfile, const timestamp& last_time) {
    if (!outfile.is_open()) return;
    outfile.seekp(sizeof(MAGIC_NUMBER) + sizeof(VERSION_NUMBER) + sizeof(timestamp));
    outfile.write(reinterpret_cast<const char*>(&last_time), sizeof(last_time));
}