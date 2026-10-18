#include "light_sensor.h"
#include "config.h"

namespace {
uint16_t lastRaw = 0;
bool dark = false;
uint32_t lastSampleMs = 0;
}  // namespace

namespace LightSensor {
void begin() {
    analogReadResolution(12);  // raw code 0..4095
    analogSetPinAttenuation(Config::LDR_GPIO, ADC_11db);
}

bool sample(uint32_t nowMs) {
    if (nowMs - lastSampleMs < Config::SAMPLE_PERIOD_MS) return false;
    lastSampleMs = nowMs;
    lastRaw = static_cast<uint16_t>(analogRead(Config::LDR_GPIO));
    if (lastRaw < Config::THRESHOLD_DARK) {
        dark = true;
    } else if (lastRaw > Config::THRESHOLD_LIGHT) {
        dark = false;
    }  // in between: keep the last state (hysteresis)
    return true;
}

uint16_t raw() { return lastRaw; }
bool isDark() { return dark; }
}  // namespace LightSensor
