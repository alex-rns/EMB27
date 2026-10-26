#pragma once

#include <Arduino.h>

struct Config {
    static constexpr uint8_t kLedPin = 7;
    static constexpr uint32_t kBlinkIntervalMs = 1000;
};
