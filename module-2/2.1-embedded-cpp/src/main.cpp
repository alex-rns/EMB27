#include <Arduino.h>

enum class LedState { On, Off };

struct Config {
    static constexpr uint8_t kLedPin = 7;
    static constexpr uint32_t kBlinkIntervalMs = 1000;
};

class Led {
    public:

    void init() {
        pinMode(Config::kLedPin, OUTPUT);
    }

    void set(LedState state) {
        digitalWrite(Config::kLedPin, state == LedState::On ? HIGH : LOW);
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

    if (nowMs - lastToggleMs >= Config::kBlinkIntervalMs) {
        ledState = ledState == LedState::On ? LedState::Off : LedState::On;
        led.set(ledState);
        lastToggleMs = nowMs;
    }
}
