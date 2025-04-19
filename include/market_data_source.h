#pragma once

#include <string>
#include <vector>

class MarketDataSource {
public:
    virtual ~MarketDataSource() = default;

    virtual bool connect() = 0;
    virtual void close() = 0;
    virtual void send(const std::string& message) = 0;
    virtual void recv(std::vector<std::string>& messages) = 0;
    virtual bool is_connected() const = 0;
};