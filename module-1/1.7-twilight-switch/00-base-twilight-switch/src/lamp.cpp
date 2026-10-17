#include "lamp.h"
#include "config.h"
#include "light_sensor.h"

namespace {
bool on = false;

uint8_t levelFor(bool lampOn) {
    return lampOn == Config::RELAY_ON_IS_HIGH ? HIGH : LOW;
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
    const bool want = LightSensor::isDark();
    if (want == on) return;  // write the pin only on a change
    on = want;
    digitalWrite(Config::RELAY_GPIO, levelFor(on));
    Serial.printf("[%5lu.%lu s] relay %s (%s)\n",
                  static_cast<unsigned long>(nowMs / 1000),
                  static_cast<unsigned long>(nowMs % 1000 / 100),
                  on ? "on" : "off", on ? "dark" : "light");
}

bool isOn() { return on; }
}  // namespace Lamp
