#pragma once

#include <Arduino.h>

namespace Motion {
constexpr uint32_t NEVER = UINT32_MAX;  // msSinceMotion() before the first motion

void begin();                            // INPUT_PULLDOWN: a loose PIR wire reads "no motion"
bool read(uint32_t nowMs);               // true while PIR OUT is HIGH; call it every loop
bool active();                           // the last reading
uint32_t msSinceMotion(uint32_t nowMs);  // NEVER until the first motion, 0 while active
}  // namespace Motion
