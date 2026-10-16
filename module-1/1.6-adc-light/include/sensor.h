#pragma once

#include <Arduino.h>

struct SensorReading {
    uint16_t raw = 0;
    uint16_t average = 0;
    uint32_t millivolts = 0;
};

namespace Sensor {
void begin();
SensorReading sample();
}  // namespace Sensor
