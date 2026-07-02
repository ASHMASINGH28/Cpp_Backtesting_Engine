#pragma once
#include <deque>
#include "Event.h"

class EventQueue {
public:
    void push(EventPtr e) { queue_.push_back(std::move(e)); }
    bool empty() const { return queue_.empty(); }

    EventPtr pop() {
        EventPtr e = queue_.front();
        queue_.pop_front();
        return e;
    }

private:
    std::deque<EventPtr> queue_;
};
