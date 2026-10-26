#include <Arduino.h>

enum class LedState { On, Off };

constexpr uint8_t kLedPin = 7;
constexpr uint32_t kBlinkIntervalMs = 1000;

class Led {
    public:

    void init() {
        pinMode(kLedPin, OUTPUT);
    }

    void set(LedState state) {
        digitalWrite(kLedPin, state == LedState::On ? HIGH : LOW);
    }
};

Led led;

void setup() {
    led.init();
}

void loop() {
    static uint32_t lastToggleMs = 0;
    static LedState ledState = LedState::Off;

    const uint32_t nowMs = millis();

    if (nowMs - lastToggleMs >= kBlinkIntervalMs) {
        ledState = ledState == LedState::On ? LedState::Off : LedState::On;
        led.set(ledState);
        lastToggleMs = nowMs;
    }
}
