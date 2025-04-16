#include "event_loop.h"
#include <thread>
#include <iostream>

duration_us operator"" _us(unsigned long long us) {
    return duration_us(us);
}

duration_us operator"" _ns(unsigned long long ns) {
    return std::chrono::duration_cast<duration_us>(std::chrono::nanoseconds(ns));
}

duration_us operator"" _ms(unsigned long long ms) {
    return std::chrono::duration_cast<duration_us>(std::chrono::milliseconds(ms));
}

duration_us operator"" _s(unsigned long long s) {
    return std::chrono::duration_cast<duration_us>(std::chrono::seconds(s));
}

duration_us operator"" _min(unsigned long long min) {
    return std::chrono::duration_cast<duration_us>(std::chrono::minutes(min));
}

duration_us operator"" _h(unsigned long long h) {
    return std::chrono::duration_cast<duration_us>(std::chrono::hours(h));
}

EventLoop::EventLoop(bool& running) : is_running_(running) {}

void EventLoop::schedule_event(void* key, duration_us delay, event_handler&& handler) {
    schedule_repeating_event(key, delay, 0_us, std::move(handler));
}

void EventLoop::schedule_repeating_event(void* key, duration_us delay, duration_us interval, event_handler&& handler) {
    timestamp first_execution = now + delay;
    scheduled_events_.emplace(key, ScheduledEvent{std::move(handler), interval, first_execution});
}

void EventLoop::remove_event(void* key) {
    scheduled_events_.erase(key);
}

void EventLoop::register_handler(event_handler&& handler) {
    simple_handlers_.push_back(std::move(handler));
}

void EventLoop::run() {
    while (is_running_) {
        now = std::chrono::time_point_cast<duration_us>(std::chrono::high_resolution_clock::now());
        process_scheduled_events();
        process_simple_handlers();
        std::this_thread::sleep_for(10_us);
    }
}

void EventLoop::stop() {
    is_running_ = false;
}

void EventLoop::process_scheduled_events() {
    for (auto it = scheduled_events_.begin(); it != scheduled_events_.end(); ) {
        ScheduledEvent& event = it->second;
        
        if (event.next_execution <= now) {
            event.handler();
            if (event.repeat_interval.count() > 0) {
                event.next_execution = now + event.repeat_interval;
                ++it;
            } else {
                it = scheduled_events_.erase(it);
            }
        } else {
            ++it;
        }
    }
}

void EventLoop::process_simple_handlers() {
    for (auto& handler : simple_handlers_) {
        handler();
    }
}

timestamp EventLoop::get_current_time() const {
    return now;
}