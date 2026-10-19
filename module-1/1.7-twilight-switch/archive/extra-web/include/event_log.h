#pragma once

#include <Arduino.h>

struct Event {
    uint32_t atMs;
    char text[48];
};

// The last Config::EVENT_COUNT events, plus one Serial line per event.
namespace EventLog {
void add(uint32_t nowMs, const char* text);  // text is cut to 47 characters; the oldest event is dropped when full
size_t count();
const Event& get(size_t i);  // 0 = the oldest
}  // namespace EventLog
