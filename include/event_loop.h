#pragma once

#include "market_data_time.h"

#include <vector>
#include <functional>
#include <chrono>
#include <map>

using event_handler = std::function<void()>;

struct ScheduledEvent {
    event_handler handler;
    duration_us repeat_interval;
    timestamp next_execution;
};

class EventLoop {
public:
    EventLoop(bool& running);

    // Disable copy semantics
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    // Disable move semantics
    EventLoop(EventLoop&&) = delete;
    EventLoop& operator=(EventLoop&&) = delete;
    
    void schedule_event(void* key, duration_us delay, event_handler&& handler);
    void schedule_repeating_event(void* key, duration_us delay, duration_us interval, event_handler&& handler);
    void register_handler(event_handler&& handler);
    void run();
    void stop();
    void remove_event(void* key);

    timestamp get_current_time() const;

private:
    std::multimap<void*, ScheduledEvent> scheduled_events_;
    std::vector<event_handler> simple_handlers_; 
    bool& is_running_;
    timestamp now;

    void process_scheduled_events();
    void process_simple_handlers();
};