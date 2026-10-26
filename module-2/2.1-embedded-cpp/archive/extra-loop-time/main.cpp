#include <Arduino.h>
#include "config.h"
#include "led.h"

Led led;

void setup() {
    Serial.begin(Config::kSerialBaud);
    led.init();
}

void loop() {
    static uint32_t lastToggleMs = 0;
    static LedState ledState = LedState::Off;

    static uint32_t loopCount = 0;
    static uint32_t loopTimeSumUs = 0;

    const uint32_t loopStartUs = micros();
    const uint32_t nowMs = millis();

    if (nowMs - lastToggleMs >= Config::kBlinkIntervalMs) {
        ledState = ledState == LedState::On ? LedState::Off : LedState::On;
        led.set(ledState);
        lastToggleMs = nowMs;
    }

    loopTimeSumUs += micros() - loopStartUs;
    loopCount++;

    if (loopCount == Config::kIterationsPerReport) {
        const float avgLoopUs = static_cast<float>(loopTimeSumUs) / loopCount;
        Serial.print("avg loop: ");
        Serial.print(avgLoopUs, 3);
        Serial.println(" us");
        loopCount = 0;
        loopTimeSumUs = 0;
    }
}
