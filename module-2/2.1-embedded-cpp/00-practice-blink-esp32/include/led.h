#pragma once

#include <Arduino.h>
#include "config.h"

enum class LedState { On, Off };

class Led {
    public:

    void init() {
        pinMode(Config::kLedPin, OUTPUT);
    }

    void set(LedState state) {
        digitalWrite(Config::kLedPin, state == LedState::On ? HIGH : LOW);
    }
};
