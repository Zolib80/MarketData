#include <iostream>

#include "bybit_market_data_feed.h"
#include "event_loop.h" 


int main() {
    std::cout << "Application started\n";
    std::map<std::string, Instrument> instrument_map = {
        {"BTCUSDT", Instrument{"BTCUSDT"}},
        {"ETHUSDT", Instrument{"ETHUSDT"}},
        {"XRPUSDT", Instrument{"XRPUSDT"}}
    };
    
    EventLoop event_loop;
    BybitMarketDataFeed marketDataFeed(event_loop, {&instrument_map["BTCUSDT"], &instrument_map["ETHUSDT"], &instrument_map["XRPUSDT"]});

    event_loop.register_handler([&marketDataFeed]() {
        marketDataFeed.run();
    });

    std::cout << "Starting Event Loop on main thread.\n";
    event_loop.run();
    std::cout << "Event Loop finished.\n";

    return 0;
}