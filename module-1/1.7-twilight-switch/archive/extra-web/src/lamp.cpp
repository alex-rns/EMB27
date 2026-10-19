#include "lamp.h"
#include "config.h"
#include "event_log.h"
#include "light_sensor.h"
#include "motion.h"

namespace {
bool on = false;
Mode currentMode = Mode::Auto;

uint8_t levelFor(bool lampOn) {
    return lampOn == Config::RELAY_ON_IS_HIGH ? HIGH : LOW;
}

void switchTo(bool lampOn) {
    on = lampOn;
    digitalWrite(Config::RELAY_GPIO, levelFor(on));
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
    if (currentMode == Mode::On) {
        if (!on) {
            switchTo(true);
            EventLog::add(nowMs, "lamp on (mode on)");
        }
        return;
    }
    if (currentMode == Mode::Off) {
        if (on) {
            switchTo(false);
            EventLog::add(nowMs, "lamp off (mode off)");
        }
        return;
    }

    // Auto. Light also switches the lamp off (the room light came on); "light" is reported before "no motion".
    // Risk: if the strip's own light reaches the LDR, on -> light -> off -> dark -> motion -> on can loop.
    if (!on) {
        if (LightSensor::isDark() && Motion::active()) {
            switchTo(true);
            EventLog::add(nowMs, "lamp on (dark + motion)");
        }
    } else if (!LightSensor::isDark()) {
        switchTo(false);
        EventLog::add(nowMs, "lamp off (light)");
    } else if (Motion::msSinceMotion(nowMs) >= Config::MOTION_TIMEOUT_MS) {
        switchTo(false);
        char text[sizeof(Event::text)];
        snprintf(text, sizeof text, "lamp off (no motion %lu s)",
                 static_cast<unsigned long>(Config::MOTION_TIMEOUT_MS / 1000));
        EventLog::add(nowMs, text);
    }
}

bool isOn() { return on; }

uint32_t secondsLeft(uint32_t nowMs) {
    if (currentMode != Mode::Auto || !on) return 0;
    const uint32_t since = Motion::msSinceMotion(nowMs);
    if (since >= Config::MOTION_TIMEOUT_MS) return 0;
    return (Config::MOTION_TIMEOUT_MS - since + 999) / 1000;
}

void setMode(Mode m, uint32_t nowMs) {
    if (m == currentMode) return;
    currentMode = m;
    char text[sizeof(Event::text)];
    snprintf(text, sizeof text, "mode -> %s", modeName(m));
    EventLog::add(nowMs, text);
    update(nowMs);  // applies at once: back in Auto the Auto rule decides right now
}

Mode mode() { return currentMode; }

const char* modeName(Mode m) {
    switch (m) {
        case Mode::On: return "on";
        case Mode::Off: return "off";
        case Mode::Auto: break;
    }
    return "auto";
}
}  // namespace Lamp
