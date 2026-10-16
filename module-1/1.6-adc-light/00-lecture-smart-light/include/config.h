#pragma once

#include <Arduino.h>

namespace Config {
// ESP32-S3-specific GPIO choices; do not copy to the original ESP32.
constexpr uint8_t kLdrGpio = 4;       // ESP32-S3 ADC1 channel 3
constexpr uint8_t kLedGpio = 7;       // External LED, series 220-ohm resistor

constexpr uint32_t kBaud = 115200;
constexpr uint32_t kSamplePeriodMs = 200;
constexpr size_t kAverageWindow = 8;

// Demo values. Calibrate from real readings on YOUR breadboard.
// With 3.3V -> LDR -> GPIO4 -> 10k -> GND:
// dark => low ADC code, bright => high ADC code.
constexpr uint16_t kLedOnBelow = 1600;
constexpr uint16_t kLedOffAbove = 2200;
static_assert(kLedOnBelow < kLedOffAbove, "Invalid hysteresis thresholds");
static_assert(kAverageWindow > 0, "Window cannot be empty");
}  // namespace Config
