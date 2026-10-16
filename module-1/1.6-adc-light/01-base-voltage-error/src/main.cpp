#include <Arduino.h>

// LDR divider on GPIO 4: 3V3 -> LDR -> GPIO 4 -> 10 kOhm -> GND.
constexpr uint8_t LDR_PIN = 4;              // ADC1
constexpr uint16_t ADC_MAX = 4095;          // 12 bits
constexpr uint32_t U_REF_MV = 3100;         // full scale at 11 dB, from the assignment
constexpr uint32_t SAMPLE_PERIOD_MS = 100;

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    analogSetPinAttenuation(LDR_PIN, ADC_11db);
}

void loop() {
    static uint32_t lastSampleMs = 0;
    const uint32_t nowMs = millis();
    if (nowMs - lastSampleMs < SAMPLE_PERIOD_MS) return;
    lastSampleMs = nowMs;

    const uint32_t raw = analogRead(LDR_PIN);
    const uint32_t uMeasMv = analogReadMilliVolts(LDR_PIN);
    const float uCalcMv = static_cast<float>(raw) / ADC_MAX * U_REF_MV;

    Serial.printf("RAW:%5lu | Ucalc:%7.1f mV | Umeas:%5lu mV | ",
                  static_cast<unsigned long>(raw), uCalcMv,
                  static_cast<unsigned long>(uMeasMv));

    if (uMeasMv == 0) {
        Serial.println("error:    n/a");
        return;
    }
    const float errorPct = (uCalcMv - uMeasMv) / uMeasMv * 100;
    Serial.printf("error:%7.2f %%\n", errorPct);
}
