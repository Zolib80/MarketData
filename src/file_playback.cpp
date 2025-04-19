#include "file_playback.h"
#include "message_recorder.h"

#include <iostream>
#include <fstream>
#include <sstream>

FilePlayback::FilePlayback(EventLoop& event_loop, const std::string& filename) 
    : event_loop_(event_loop) 
    , filename_(filename) 
{
    infile_.open(filename_, std::ios::binary);
    if (!infile_.is_open()) {
        std::cerr << "Error opening file for playback: " << filename_ << '\n';
        event_loop_.stop();
        return;
    }

    uint32_t magic_number_read;
    uint8_t version_number_read;
    timestamp first_message_time_read;
    timestamp last_message_time_read;

    infile_.read(reinterpret_cast<char*>(&magic_number_read), sizeof(magic_number_read));

    if (magic_number_read != MAGIC_NUMBER) {
        std::cerr << "Error: Invalid magic number in playback file.\n";
        close();
        return;
    }

    infile_.read(reinterpret_cast<char*>(&version_number_read), sizeof(version_number_read));

    if (version_number_read != VERSION_NUMBER) {
        std::cerr << "Error: Incompatible version number in playback file.\n";
        close();
        return;
    }

    infile_.read(reinterpret_cast<char*>(&first_message_time_read), sizeof(first_message_time_read));
    infile_.read(reinterpret_cast<char*>(&last_message_time_read), sizeof(last_message_time_read));
    infile_.read(reinterpret_cast<char*>(&next_message_time_), sizeof(next_message_time_));

    first_message_time_ = first_message_time_read;
    is_connected_ = true;
    std::cout << "Connected to playback file: " << filename_ << '\n';
}

FilePlayback::~FilePlayback() {
    close();
}

bool FilePlayback::connect() {
    return true;
}

void FilePlayback::close() {
    if (infile_.is_open()) {
        infile_.close();
    }
    event_loop_.stop();
    is_connected_ = false;
}

void FilePlayback::send(const std::string& message) {
    std::cout << "Message: " << message << " has been sent.\n";
    return;
}

void FilePlayback::recv(std::vector<std::string>& messages) {
    messages.clear();

    if (!infile_.is_open() || infile_.eof()) {
        close();
        return;
    }

    timestamp current_message_time;
    MessageType message_type;
    uint32_t message_length;
    std::string message;

    current_message_time = next_message_time_;
    while (!infile_.eof() && current_message_time == next_message_time_) {
        infile_.read(reinterpret_cast<char*>(&message_type), sizeof(message_type));
        infile_.read(reinterpret_cast<char*>(&message_length), sizeof(message_length));
        message.resize(message_length);
        infile_.read(message.data(), message_length);
    
        if (infile_.gcount() == message_length) {
            if (message_type == MessageType::Incoming) {
                messages.push_back(message);
            }
        } else if (infile_.gcount() > 0) {
            close();
            std::cerr << "Error reading message from playback file (incomplete read).\n";
        }
        if (!infile_.eof()) {
            infile_.read(reinterpret_cast<char*>(&next_message_time_), sizeof(next_message_time_));
        }
    };
}
