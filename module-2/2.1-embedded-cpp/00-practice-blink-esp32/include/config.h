#pragma once

#include <Arduino.h>

struct Config {
    static constexpr uint8_t kLedPin = 7;
    static constexpr uint8_t kBtnPin = 9;
    static constexpr uint32_t kBlinkIntervalMs = 1000;

    static constexpr uint32_t kSerialBaud = 115200;
    static constexpr uint32_t kIterationsPerReport = 1000;

    static constexpr uint32_t kDebounceMs = 30;
};
