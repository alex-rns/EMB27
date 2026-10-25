#include <Arduino.h>

enum class LedState { On, Off };
constexpr uint8_t kLedPin = 7;
constexpr uint32_t kBlinkIntervalMs = 1000;

void setup() {
    pinMode(kLedPin, OUTPUT);
}

void loop() {
    digitalWrite(kLedPin, HIGH);
    delay(kBlinkIntervalMs);
    digitalWrite(kLedPin, LOW);
    delay(kBlinkIntervalMs);
}
