#include "file_playback.h"
#include "message_recorder.h"

#include <iostream>
#include <fstream>
#include <sstream>

FilePlayback::FilePlayback(const std::string& filename) : filename_(filename) {}

FilePlayback::~FilePlayback() {
    close();
}

bool FilePlayback::connect() {
    infile_.open(filename_, std::ios::binary);
    if (!infile_.is_open()) {
        std::cerr << "Error opening file for playback: " << filename_ << '\n';
        return false;
    }

    uint32_t magic_number_read;
    uint8_t version_number_read;
    timestamp first_message_time_read;
    timestamp last_message_time_read;

    infile_.read(reinterpret_cast<char*>(&magic_number_read), sizeof(magic_number_read));
    infile_.read(reinterpret_cast<char*>(&version_number_read), sizeof(version_number_read));
    infile_.read(reinterpret_cast<char*>(&first_message_time_read), sizeof(first_message_time_read));
    infile_.read(reinterpret_cast<char*>(&last_message_time_read), sizeof(last_message_time_read));

    if (magic_number_read != MAGIC_NUMBER) {
        std::cerr << "Error: Invalid magic number in playback file.\n";
        close();
        return false;
    }
    if (version_number_read != VERSION_NUMBER) {
        std::cerr << "Error: Incompatible version number in playback file.\n";
        close();
        return false;
    }

    first_message_time_ = first_message_time_read;
    is_connected_ = true;
    std::cout << "Connected to playback file: " << filename_ << '\n';
    is_running_ = true; 
    return true;
}

void FilePlayback::close() {
    if (infile_.is_open()) {
        infile_.close();
    }
    is_connected_ = false;
    is_running_ = false; 
}

void FilePlayback::send(const std::string& message) {
    std::cout << "Message: " << message << " has been sent.\n";
    return;
}

void FilePlayback::recv(std::vector<std::string>& messages) {
    messages.clear();

    if (!infile_.is_open() || infile_.eof()) {
        is_running_ = false;
        close();
        return;
    }

    timestamp current_message_time;
    MessageType message_type;
    uint32_t message_length;
    std::string message;

    infile_.read(reinterpret_cast<char*>(&current_message_time), sizeof(current_message_time));
    infile_.read(reinterpret_cast<char*>(&message_type), sizeof(message_type));
    infile_.read(reinterpret_cast<char*>(&message_length), sizeof(message_length));
    message.resize(message_length);
    infile_.read(message.data(), message_length);

    if (infile_.gcount() == message_length) {
        messages.push_back(message);
        if (previous_message_time_.time_since_epoch().count() != 0) {
            std::this_thread::sleep_for(current_message_time - previous_message_time_);
        }
        previous_message_time_ = current_message_time;
    } else if (infile_.gcount() > 0) {
        close();
        std::cerr << "Error reading message from playback file (incomplete read).\n";
    }
}
