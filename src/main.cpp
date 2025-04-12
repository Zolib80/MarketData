#include <iostream>

#include "web_socket_handler.h"
#include "event_loop.h" 


EventLoop event_loop;

int main() {
    std::cout << "Application started\n";

    BybitMarketDataFeed marketDataFeed(event_loop);

    event_loop.register_handler([&marketDataFeed]() {
        marketDataFeed.run();
    });

    std::cout << "Starting Event Loop on main thread.\n";
    event_loop.run();
    std::cout << "Event Loop finished.\n";

    return 0;
}