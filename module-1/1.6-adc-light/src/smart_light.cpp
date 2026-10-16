#include "smart_light.h"
#include "config.h"

namespace {
bool ledOn = false;
}  // namespace

namespace SmartLight {
void begin() {
    pinMode(Config::kLedGpio, OUTPUT);
    digitalWrite(Config::kLedGpio, LOW);
}

void update(uint16_t averagedAdc) {
    if (!ledOn && averagedAdc < Config::kLedOnBelow) {
        ledOn = true;
        digitalWrite(Config::kLedGpio, ledOn);
    } else if (ledOn && averagedAdc > Config::kLedOffAbove) {
        ledOn = false;
        digitalWrite(Config::kLedGpio, ledOn);
    }
}

bool isOn() { return ledOn; }
}  // namespace SmartLight
