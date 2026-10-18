#include "event_log.h"
#include "config.h"

namespace {
Event events[Config::EVENT_COUNT];
size_t used = 0;
}  // namespace

namespace EventLog {
void add(uint32_t nowMs, const char* text) {
    if (used == Config::EVENT_COUNT) {
        for (size_t i = 1; i < used; i++) events[i - 1] = events[i];  // drop the oldest
        used--;
    }
    Event& e = events[used++];
    e.atMs = nowMs;
    snprintf(e.text, sizeof e.text, "%s", text);  // truncates and always terminates

    Serial.printf("[%5lu.%lu s] %s\n", static_cast<unsigned long>(nowMs / 1000),
                  static_cast<unsigned long>(nowMs % 1000 / 100), e.text);
}

size_t count() { return used; }

const Event& get(size_t i) { return events[i]; }
}  // namespace EventLog
