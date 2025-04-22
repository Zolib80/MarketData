#pragma once

#include <cstdint>

enum class MessageType : uint8_t {
    Connect = 0,
    Disconnect,
    Incoming,
    Outgoing,
};