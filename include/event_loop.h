#pragma once

#include <vector>
#include <functional>
#include <chrono>
#include <map>

using event_handler = std::function<void()>;
using timestamp = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::microseconds>;
using duration_us = std::chrono::microseconds;

// Declare literal operators
duration_us operator"" _us(unsigned long long us);
duration_us operator"" _ns(unsigned long long ns);
duration_us operator"" _ms(unsigned long long ms);
duration_us operator"" _s(unsigned long long s);
duration_us operator"" _min(unsigned long long min);
duration_us operator"" _h(unsigned long long h);

struct ScheduledEvent {
    event_handler handler;
    duration_us repeat_interval;
    timestamp next_execution;
};

class EventLoop {
public:
    void schedule_event(void* key, duration_us delay, event_handler&& handler);
    void schedule_repeating_event(void* key, duration_us delay, duration_us interval, event_handler&& handler);
    void register_handler(event_handler&& handler);
    void run();
    void stop();
    void remove_event(void* key);

private:
    std::multimap<void*, ScheduledEvent> scheduled_events_;
    std::vector<event_handler> simple_handlers_; 
    bool is_running_ = false;

    void process_scheduled_events();
    void process_simple_handlers();
};