#include "lamp.h"
#include "config.h"
#include "light_sensor.h"
#include "motion.h"

namespace {
bool on = false;

uint8_t levelFor(bool lampOn) {
    return lampOn == Config::RELAY_ON_IS_HIGH ? HIGH : LOW;
}

void switchTo(bool lampOn, uint32_t nowMs) {
    on = lampOn;
    digitalWrite(Config::RELAY_GPIO, levelFor(on));
    Serial.printf("[%5lu.%lu s] lamp %s ", static_cast<unsigned long>(nowMs / 1000),
                  static_cast<unsigned long>(nowMs % 1000 / 100), on ? "on" : "off");
}
}  // namespace

namespace Lamp {
void begin() {
    // The off level goes in before OUTPUT, so an inverted relay doesn't click at boot.
    digitalWrite(Config::RELAY_GPIO, levelFor(false));
    pinMode(Config::RELAY_GPIO, OUTPUT);
    digitalWrite(Config::RELAY_GPIO, levelFor(false));
}

void update(uint32_t nowMs) {
    // Light only prevents switching on: the lamp's own light on the LDR must not switch it off.
    if (!on) {
        if (LightSensor::isDark() && Motion::active()) {
            switchTo(true, nowMs);
            Serial.println("(dark + motion)");
        }
    } else if (Motion::msSinceMotion(nowMs) >= Config::MOTION_TIMEOUT_MS) {
        switchTo(false, nowMs);
        Serial.printf("(no motion %lu s)\n", static_cast<unsigned long>(Config::MOTION_TIMEOUT_MS / 1000));
    }
}

bool isOn() { return on; }

uint32_t secondsLeft(uint32_t nowMs) {
    if (!on) return 0;
    const uint32_t since = Motion::msSinceMotion(nowMs);
    if (since >= Config::MOTION_TIMEOUT_MS) return 0;
    return (Config::MOTION_TIMEOUT_MS - since + 999) / 1000;
}
}  // namespace Lamp
