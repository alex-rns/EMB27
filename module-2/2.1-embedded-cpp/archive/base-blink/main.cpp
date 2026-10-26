#include <Arduino.h>
#include "config.h"
#include "led.h"

Led led;

void setup() {
    led.init();
}

void loop() {
    static uint32_t lastToggleMs = 0;
    static LedState ledState = LedState::Off;

    const uint32_t nowMs = millis();

    if (nowMs - lastToggleMs >= Config::kBlinkIntervalMs) {
        ledState = ledState == LedState::On ? LedState::Off : LedState::On;
        led.set(ledState);
        lastToggleMs = nowMs;
    }
}
