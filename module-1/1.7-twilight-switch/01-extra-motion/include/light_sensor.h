#pragma once

#include <Arduino.h>

namespace LightSensor {
void begin();
bool sample(uint32_t nowMs);  // reads once per SAMPLE_PERIOD_MS; true when it read
uint16_t raw();
bool isDark();                // hysteresis state; starts as light
}  // namespace LightSensor
