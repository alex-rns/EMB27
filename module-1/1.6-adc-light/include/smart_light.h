#pragma once

#include <Arduino.h>

namespace SmartLight {
void begin();
void update(uint16_t averagedAdc);
bool isOn();
}  // namespace SmartLight
