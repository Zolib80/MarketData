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

    bool try_write(timestamp ts, MessageType type, std::string_view message);
    bool try_read(std::function<void(timestamp, MessageType, std::string_view)> processor);
    size_t get_dropped_message_count() const { return dropped_message_count.load(std::memory_order_relaxed); }

private:
    std::unique_ptr<std::byte[]> buffer_;
    size_t capacity_;
    std::atomic<size_t> write_index{0};
    std::atomic<size_t> read_index{0};
    std::atomic<size_t> dropped_message_count{0};

    static constexpr size_t HEADER_SIZE = sizeof(QueueHeader);
};