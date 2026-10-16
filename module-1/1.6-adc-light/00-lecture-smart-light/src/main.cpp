#include <Arduino.h>
#include "config.h"
#include "sensor.h"
#include "smart_light.h"

void setup() {
    Serial.begin(Config::kBaud);
    SmartLight::begin();
    Sensor::begin();
}

void loop() {
    static uint32_t lastSampleMs = 0;
    const uint32_t nowMs = millis();
    if (nowMs - lastSampleMs < Config::kSamplePeriodMs) return;
    lastSampleMs = nowMs;

    const SensorReading reading = Sensor::sample();
    SmartLight::update(reading.average);

    // Serial Plotter: separate curves with meaningful labels.
    Serial.printf("raw:%u\t  avg:%u\t  mV:%lu\t  led:%u\n",
                  static_cast<unsigned>(reading.raw),
                  static_cast<unsigned>(reading.average),
                  static_cast<unsigned long>(reading.millivolts),
                  SmartLight::isOn() ? 1U : 0U);
}
