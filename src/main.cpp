#include <iostream>

#include "bybit_market_data_feed.h"
#include "event_loop.h" 

#include <csignal>
#include <atomic>

bool g_running = true;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nCtrl+C received. Shutting down gracefully...\n";
        g_running = false;
    }
}

int main() {
    std::cout << "Application started\n";
    std::map<std::string, Instrument> instrument_map = {
        {"BTCUSDT", Instrument{"BTCUSDT"}},
        {"ETHUSDT", Instrument{"ETHUSDT"}},
        {"XRPUSDT", Instrument{"XRPUSDT"}}
    };
    
    std::signal(SIGINT, signal_handler);
    
    EventLoop event_loop(g_running);

    auto now = std::chrono::system_clock::now();
    std::string filename = std::format("bybit_live_{:%Y%m%d_%H%M%S}.zbmd", now);
    std::cout << "Recording to file: " << filename << '\n';

    std::unique_ptr<MessageRecorder> recorder_ptr = std::make_unique<MessageRecorder>(filename);

    BybitMarketDataFeed marketDataFeed(
        event_loop,
        {&instrument_map["BTCUSDT"], &instrument_map["ETHUSDT"], &instrument_map["XRPUSDT"]},
        recorder_ptr.get()
    );

    event_loop.register_handler([&marketDataFeed]() {
        marketDataFeed.run();
    });

    std::cout << "Starting Event Loop on main thread.\n";
    event_loop.run();
    std::cout << "Event Loop finished.\n";

    return 0;
}