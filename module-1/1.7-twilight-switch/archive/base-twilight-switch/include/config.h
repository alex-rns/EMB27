#pragma once

#include <Arduino.h>

namespace Config {
// ESP32-S3 pins. GPIO 4 is on ADC1, which keeps working while Wi-Fi is on.
constexpr uint8_t LDR_GPIO = 4;    // midpoint of the LDR + 10 kOhm divider
constexpr uint8_t RELAY_GPIO = 7;  // "GPIO Out": 10 kOhm -> BC547 base -> relay module IN

constexpr uint32_t BAUD = 115200;
constexpr uint32_t SAMPLE_PERIOD_MS = 100;
constexpr uint32_t STATUS_PERIOD_MS = 1000;

// Start values from lesson 1.6; set the real ones from Serial readings on this breadboard.
// 3V3 -> LDR -> GPIO 4 -> 10 kOhm -> GND: dark => low code, bright => high code.
constexpr uint16_t THRESHOLD_DARK = 1600;   // below: dark
constexpr uint16_t THRESHOLD_LIGHT = 2200;  // above: light; in between: no change
static_assert(THRESHOLD_DARK < THRESHOLD_LIGHT, "Invalid hysteresis thresholds");
static_assert(THRESHOLD_LIGHT <= 4095, "A 12-bit ADC reads 0..4095");

// GPIO level that switches the relay on. The BC547 stage inverts and a low-level-trigger
// module inverts again, so HIGH = relay on. A high-level-trigger module needs false.
constexpr bool RELAY_ON_IS_HIGH = true;
}  // namespace Config
