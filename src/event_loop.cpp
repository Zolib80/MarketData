#include "event_loop.h"
#include <thread>

void EventLoop::schedule_event(void* key, duration_us delay, const event_handler& handler) {
    schedule_repeating_event(key, delay, 0_us, handler);
}

void EventLoop::schedule_repeating_event(void* key, duration_us delay, duration_us interval, const event_handler& handler) {
    timestamp now = std::chrono::time_point_cast<duration_us>(std::chrono::high_resolution_clock::now());
    timestamp first_execution = now + delay;
    scheduled_events_.emplace(key, ScheduledEvent{handler, interval, first_execution});
}

void EventLoop::remove_event(void* key) {
    scheduled_events_.erase(key);
}

void EventLoop::register_handler(const event_handler& handler) {
    simple_handlers_.push_back(handler);
}

void EventLoop::run() {
    is_running_ = true;
    while (is_running_) {
        process_scheduled_events();
        process_simple_handlers();
        std::this_thread::sleep_for(10_us);
    }
}

void EventLoop::stop() {
    is_running_ = false;
}

void EventLoop::process_scheduled_events() {
    timestamp now = std::chrono::time_point_cast<duration_us>(std::chrono::high_resolution_clock::now());

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

void EventLoop::process_simple_handlers() const {
    for (const auto& handler : simple_handlers_) {
        handler();
    }
}