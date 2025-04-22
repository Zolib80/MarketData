#include "spsc_byte_ring_buffer.h"

#include <cstring>
#include <iostream>
#include <thread>

SpscByteRingBuffer::SpscByteRingBuffer(size_t capacity) :
    buffer_(std::make_unique<std::byte[]>(capacity)),
    capacity_(capacity) {}

bool SpscByteRingBuffer::try_write(timestamp ts, MessageType type, std::string_view message) {
    size_t message_length = message.length();
    size_t entry_size = HEADER_SIZE + message_length;
    size_t current_read_index = read_index.load(std::memory_order_acquire);
    size_t current_write_index = write_index.load(std::memory_order_relaxed);
    size_t next_write_index = (current_write_index + entry_size) % capacity_;

    // Ha a következő írási pozíció megegyezik az olvasási pozícióval, a puffer tele van.
    // Felülírjuk a legrégebbi adatot.
    if (next_write_index == current_read_index) {
        dropped_message_count.fetch_add(1, std::memory_order_relaxed);
        // Nem kell a read_index-et mozgatni, mert az olvasó úgyis el fogja olvasni a felülírt részt előbb-utóbb.
    } else {
        // Ellenőrizzük, hogy van-e elegendő hely az íráshoz.
        // Mivel SPSC, nem kell bonyolultabban ellenőrizni a wrap-aroundot,
        // egyszerűen megnézzük, hogy a következő írási pozíció nem "előzi-e be" az olvasási pozíciót.
        size_t available_space;
        if (current_write_index >= current_read_index) {
            available_space = capacity_ - current_write_index + current_read_index - 1;
        } else {
            available_space = current_read_index - current_write_index - 1;
        }

        if (entry_size > capacity_ - 1) {
            // Az üzenet túl nagy a pufferhez
            return false;
        }

        if (entry_size > available_space) {
            dropped_message_count.fetch_add(1, std::memory_order_relaxed);
            // Nincs elég hely, felülírjuk a legrégebbi adatot (nem csinálunk semmit itt,
            // mert az írás alább megtörténik a jelenlegi write_index-re).
        }
    }

    QueueHeader header{ts, type, static_cast<uint32_t>(message_length)};

    // Írjuk a fejlécet
    std::memcpy(buffer_.get() + current_write_index, &header, HEADER_SIZE);

    // Írjuk az üzenetadatot
    std::memcpy(buffer_.get() + current_write_index + HEADER_SIZE, message.data(), message_length);

    // Frissítsük a write_index-et
    write_index.store(next_write_index, std::memory_order_release);
    return true;
}

bool SpscByteRingBuffer::try_read(std::function<void(timestamp, MessageType, std::string_view)> processor) {
    size_t current_read_index = read_index.load(std::memory_order_relaxed);
    size_t current_write_index = write_index.load(std::memory_order_acquire);

    if (current_read_index == current_write_index) {
        return false; // Puffer üres
    }

    QueueHeader header;
    std::memcpy(&header, buffer_.get() + current_read_index, HEADER_SIZE);

    size_t message_length = header.message_length;
    size_t entry_size = HEADER_SIZE + message_length;
    size_t next_read_index = (current_read_index + entry_size) % capacity_;

    std::string_view message(reinterpret_cast<const char*>(buffer_.get() + current_read_index + HEADER_SIZE), message_length);

    processor(header.timestamp, header.message_type, message);

    read_index.store(next_read_index, std::memory_order_release);
    return true;
}
