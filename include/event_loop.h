#include <vector>
#include <functional>
#include <chrono>
#include <map>

using event_handler = std::function<void()>;
using timestamp = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::microseconds>;
using duration_us = std::chrono::microseconds;

inline duration_us operator"" _us(unsigned long long us) {
    return duration_us(us);
}

inline duration_us operator"" _ns(unsigned long long ns) {
    return std::chrono::duration_cast<duration_us>(std::chrono::nanoseconds(ns));
}

inline duration_us operator"" _ms(unsigned long long ms) {
    return std::chrono::duration_cast<duration_us>(std::chrono::milliseconds(ms));
}

inline duration_us operator"" _s(unsigned long long s) {
    return std::chrono::duration_cast<duration_us>(std::chrono::seconds(s));
}

inline duration_us operator"" _min(unsigned long long min) {
    return std::chrono::duration_cast<duration_us>(std::chrono::minutes(min));
}

inline duration_us operator"" _h(unsigned long long h) {
    return std::chrono::duration_cast<duration_us>(std::chrono::hours(h));
}

struct ScheduledEvent {
    event_handler handler;
    duration_us repeat_interval;
    timestamp next_execution;
};

class EventLoop {
public:
    void schedule_event(void* key, duration_us delay, const event_handler& handler);
    void schedule_repeating_event(void* key, duration_us delay, duration_us interval, const event_handler& handler);
    void register_handler(const event_handler& handler);
    void run();
    void stop();
    void remove_event(void* key);

private:
    std::multimap<void*, ScheduledEvent> scheduled_events_;
    std::vector<event_handler> simple_handlers_; 
    bool is_running_ = false;

    void process_scheduled_events();
    void process_simple_handlers() const;
};