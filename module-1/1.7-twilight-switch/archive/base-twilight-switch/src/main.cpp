#include <Arduino.h>
#include "config.h"
#include "lamp.h"
#include "light_sensor.h"

void setup() {
    Serial.begin(Config::BAUD);
    Lamp::begin();  // the relay pin is defined (off) before anything else
    LightSensor::begin();
}

void loop() {
    static uint32_t lastStatusMs = 0;
    const uint32_t nowMs = millis();

    LightSensor::sample(nowMs);
    Lamp::update(nowMs);

    if (nowMs - lastStatusMs >= Config::STATUS_PERIOD_MS) {
        lastStatusMs = nowMs;
        Serial.printf("raw: %u | %s | relay: %s\n",
                      static_cast<unsigned>(LightSensor::raw()),
                      LightSensor::isDark() ? "dark" : "light",
                      Lamp::isOn() ? "on" : "off");
    }
}
