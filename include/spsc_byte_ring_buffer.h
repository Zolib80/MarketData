#pragma once

#include "market_data_time.h"
#include "market_data_constants.h"
#include "message_type.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <atomic>
#include <string_view>

#pragma pack(push, 1)
struct QueueHeader {
    timestamp timestamp;
    MessageType message_type;
    uint32_t message_length;
};
#pragma pack(pop)

class SpscByteRingBuffer {
public:
    SpscByteRingBuffer(size_t capacity);
    ~SpscByteRingBuffer() = default;

    void write(timestamp ts, MessageType type, std::string_view message);
    std::optional<std::pair<QueueHeader, std::string_view>> read_next(size_t& current_read_offset);

private:
    std::unique_ptr<std::byte[]> buffer_;
    size_t capacity_;
    std::atomic<size_t> write_index{0};

    static constexpr size_t HEADER_SIZE = sizeof(QueueHeader);
};