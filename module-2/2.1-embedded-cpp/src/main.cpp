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
    led.set(LedState::On);
    delay(kBlinkIntervalMs);
    led.set(LedState::Off);
    delay(kBlinkIntervalMs);
}
