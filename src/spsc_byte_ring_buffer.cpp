#include "spsc_byte_ring_buffer.h"

#include <cstring>
#include <iostream>
#include <thread>

SpscByteRingBuffer::SpscByteRingBuffer(size_t capacity) 
  : buffer_(std::make_unique<std::byte[]>(capacity))
  , capacity_(capacity) 
{}

void SpscByteRingBuffer::write(timestamp ts, MessageType type, std::string_view message) {
    size_t message_length = message.length();
    size_t entry_size = HEADER_SIZE + message_length;
    size_t current_write_index = write_index.load(std::memory_order_relaxed);
    size_t next_write_index = (current_write_index + entry_size) % capacity_;

    QueueHeader header{ts, type, static_cast<uint32_t>(message_length)};

    std::memcpy(buffer_.get() + current_write_index, &header, HEADER_SIZE);
    std::memcpy(buffer_.get() + current_write_index + HEADER_SIZE, message.data(), message_length);
    
    write_index.store(next_write_index, std::memory_order_release);

    return; 
}

std::optional<std::pair<QueueHeader, std::string_view>> SpscByteRingBuffer::read_next(size_t& current_read_offset) {
    size_t current_write_index = write_index.load(std::memory_order_acquire);

    if (current_read_offset == current_write_index) {
        return std::nullopt; // Puffer üres az olvasási pozícióhoz képest
    }

    QueueHeader header;
    std::memcpy(&header, buffer_.get() + current_read_offset, HEADER_SIZE);

    size_t message_length = header.message_length;
    size_t entry_size = HEADER_SIZE + message_length;

    std::string_view message(reinterpret_cast<const char*>(buffer_.get() + current_read_offset + HEADER_SIZE), message_length);

    current_read_offset = (current_read_offset + entry_size) % capacity_;
    return std::make_pair(header, message);
}
