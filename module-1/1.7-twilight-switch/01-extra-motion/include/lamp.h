#pragma once

#include <Arduino.h>

namespace Lamp {
void begin();
void update(uint32_t nowMs);           // the only code that writes the relay pin
bool isOn();
uint32_t secondsLeft(uint32_t nowMs);  // 0 when off, else seconds until the auto-off, rounded up
}  // namespace Lamp
