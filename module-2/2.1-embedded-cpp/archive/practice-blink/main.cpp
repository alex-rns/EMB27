#include <Arduino.h>
#include "config.h"
#include "led.h"

Led led;

enum class LedMode {Blink, On, Off};

LedMode nextMode(LedMode mode) {
    switch (mode) {
        case LedMode::Blink: return LedMode::On;
        case LedMode::On:    return LedMode::Off;
        case LedMode::Off:   return LedMode::Blink;
    }
    return LedMode::Blink;
}

volatile bool btnPushed = false;

void IRAM_ATTR onBtnPush() {
    btnPushed = true;
}

void setup() {
    Serial.begin(Config::kSerialBaud);
    led.init();
    pinMode(Config::kBtnPin, INPUT_PULLUP);
    attachInterrupt(Config::kBtnPin, onBtnPush, FALLING);
}

void loop() {
    static uint32_t lastToggleMs = 0;
    static LedState ledState = LedState::Off;

    static uint32_t loopCount = 0;
    static uint32_t loopTimeSumUs = 0;

    static LedMode ledMode = LedMode::Blink;
    static uint32_t lastBtnPressMs = 0;

    const uint32_t loopStartUs = micros();
    const uint32_t nowMs = millis();

    if (btnPushed) {
        btnPushed = false;
        if (nowMs - lastBtnPressMs >= Config::kDebounceMs) {
            lastBtnPressMs = nowMs;
            ledMode = nextMode(ledMode);
        }
    }

    if (ledMode == LedMode::On) {
        led.set(LedState::On);
    }
    if (ledMode == LedMode::Off) {
        led.set(LedState::Off);
    }
    if (ledMode == LedMode::Blink && nowMs - lastToggleMs >= Config::kBlinkIntervalMs) {
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
