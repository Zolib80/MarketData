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
        stop();
        return;
    }

    infile_.read(reinterpret_cast<char*>(&version_number_read), sizeof(version_number_read));

    if (version_number_read != VERSION_NUMBER) {
        std::cerr << "Error: Incompatible version number in playback file.\n";
        stop();
        return;
    }

    infile_.read(reinterpret_cast<char*>(&first_message_time_read), sizeof(first_message_time_read));
    infile_.read(reinterpret_cast<char*>(&last_message_time_read), sizeof(last_message_time_read));
    infile_.read(reinterpret_cast<char*>(&next_message_time_), sizeof(next_message_time_));

    first_message_time_ = first_message_time_read;
    std::cout << "Connected to playback file: " << filename_ << '\n';
}

FilePlayback::~FilePlayback() {
    stop();
}

void FilePlayback::connect() {
}

void FilePlayback::close() {
}

void FilePlayback::stop() {
    if (infile_.is_open()) {
        std::cout << "Closing playback file: " << filename_ << '\n';
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

    for (auto& message : upcoming_messages_) {
        switch (message.type) {
            case MessageType::Connect:
                is_connected_= true;
                break;
            case MessageType::Disconnect:
                is_connected_= false;
                break;
            case MessageType::Incoming:
                messages.push_back(message.content);
                break;
            case MessageType::Outgoing:
                break;
            default:
                std::cerr << "Error: Unknown message type in playback file.\n";
        }
    };
}

void FilePlayback::preload_messages()
{
    upcoming_messages_.clear();
    if (!infile_.is_open() || infile_.eof()) {
        stop();
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
            upcoming_messages_.emplace_back(Message{message_type, message});
        } else if (infile_.gcount() > 0) {
            stop();
            std::cerr << "Error reading message from playback file (incomplete read).\n";
        }
        if (!infile_.eof()) {
            infile_.read(reinterpret_cast<char*>(&next_message_time_), sizeof(next_message_time_));
        }
    };    

    return;
}

void FilePlayback::process_next_message()
{   
    preload_messages();

    for (auto& message : upcoming_messages_) {
        switch (message.type) {
            case MessageType::Connect:
                is_connected_= true;
                break;
            case MessageType::Disconnect:
                is_connected_= false;
                break;
            case MessageType::Incoming:
                break;
            case MessageType::Outgoing:
                break;
            default:
                std::cerr << "Error: Unknown message type in playback file.\n";
        }
    };
}
