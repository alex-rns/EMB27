#include <Arduino.h>
#include "config.h"
#include "lamp.h"
#include "light_sensor.h"
#include "motion.h"

namespace {
void printStatus(uint32_t nowMs) {
    char motion[24];
    const uint32_t since = Motion::msSinceMotion(nowMs);
    if (since == Motion::NEVER) {
        snprintf(motion, sizeof motion, "never");
    } else if (since == 0) {
        snprintf(motion, sizeof motion, "now");
    } else {
        snprintf(motion, sizeof motion, "%lu.%lu s ago", static_cast<unsigned long>(since / 1000),
                 static_cast<unsigned long>(since % 1000 / 100));
    }

    char lamp[24];
    if (Lamp::isOn()) {
        snprintf(lamp, sizeof lamp, "on, %lu s left", static_cast<unsigned long>(Lamp::secondsLeft(nowMs)));
    } else {
        snprintf(lamp, sizeof lamp, "off");
    }

    Serial.printf("raw: %u | %s | motion: %s | lamp: %s\n", static_cast<unsigned>(LightSensor::raw()),
                  LightSensor::isDark() ? "dark" : "light", motion, lamp);
}
}  // namespace

void setup() {
    Serial.begin(Config::BAUD);
    Lamp::begin();  // the relay pin is defined (off) before anything else
    LightSensor::begin();
    Motion::begin();
}

void loop() {
    static uint32_t lastStatusMs = 0;
    const uint32_t nowMs = millis();

    LightSensor::sample(nowMs);
    Motion::read(nowMs);
    Lamp::update(nowMs);

    if (nowMs - lastStatusMs >= Config::STATUS_PERIOD_MS) {
        lastStatusMs = nowMs;
        printStatus(nowMs);
    }
}
