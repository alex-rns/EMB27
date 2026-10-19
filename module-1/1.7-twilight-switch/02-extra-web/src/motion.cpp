#include "motion.h"
#include "config.h"

namespace {
bool high = false;
bool seen = false;          // any motion since boot
uint32_t lastMotionMs = 0;  // the last loop that read OUT = HIGH, i.e. the falling edge once OUT is low
}  // namespace

namespace Motion {
void begin() {
    pinMode(Config::PIR_GPIO, INPUT_PULLDOWN);
}

bool read(uint32_t nowMs) {
    high = digitalRead(Config::PIR_GPIO) == HIGH;
    if (high) {
        seen = true;
        lastMotionMs = nowMs;
    }
    return high;
}

bool active() { return high; }

uint32_t msSinceMotion(uint32_t nowMs) {
    if (!seen) return NEVER;
    if (high) return 0;
    return nowMs - lastMotionMs;
}
}  // namespace Motion
