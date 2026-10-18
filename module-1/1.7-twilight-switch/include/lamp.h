#pragma once

#include <Arduino.h>

enum class Mode : uint8_t { Auto, On, Off };

namespace Lamp {
void begin();
void update(uint32_t nowMs);           // the only code that writes the relay pin
bool isOn();
uint32_t secondsLeft(uint32_t nowMs);  // 0 when off or outside Auto, else seconds until the auto-off, rounded up

// Auto after every reset (not stored). On / Off ignore the sensors. A change is applied at once and logged.
void setMode(Mode m, uint32_t nowMs);
Mode mode();
const char* modeName(Mode m);  // "auto" / "on" / "off"
}  // namespace Lamp
