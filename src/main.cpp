#include "bybit_market_data_feed.h"
#include "event_loop.h" 
#include "file_playback.h"
#include "web_socket.h"

#include <iostream>
#include <csignal>

bool g_running = true;
bool is_replay = false;

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

    std::string filename;
    std::unique_ptr<MarketDataSource> data_source_ptr;
    std::unique_ptr<MessageRecorder> recorder_ptr = nullptr;
    
    if (is_replay) {
        filename = std::format("bybit_live_20250417_150141.977928174.zbmd");
        data_source_ptr = std::make_unique<FilePlayback>(event_loop, filename);
    } else {
        auto now = std::chrono::system_clock::now();
        filename = std::format("bybit_live_{:%Y%m%d_%H%M%S}.zbmd", now);
        std::cout << "Recording to file: " << filename << '\n';
        data_source_ptr = std::make_unique<WebSocket>(BybitMarketDataFeed::BASE_URL);
        recorder_ptr = std::make_unique<MessageRecorder>(filename);
    }

    BybitMarketDataFeed marketDataFeed(
        event_loop,
        data_source_ptr.get(),
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