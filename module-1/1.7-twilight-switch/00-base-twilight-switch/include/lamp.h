#pragma once

#include <Arduino.h>

namespace Lamp {
void begin();
void update(uint32_t nowMs);  // the only code that writes the relay pin
bool isOn();
}  // namespace Lamp
