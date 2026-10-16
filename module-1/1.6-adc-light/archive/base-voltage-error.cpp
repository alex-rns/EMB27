#include <Arduino.h>

#define LDR_PIN 4
#define ADC_MAX 4095
#define U_REF_MV 3100

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    analogSetPinAttenuation(LDR_PIN, ADC_11db);
}

void loop() {
    const uint32_t raw = analogRead(LDR_PIN);
    const uint32_t Umeas = analogReadMilliVolts(LDR_PIN);

    const float Ucalc = static_cast<float>(raw) / ADC_MAX * U_REF_MV;
    const float error = (Ucalc - Umeas) / Umeas * 100;

    Serial.printf("RAW:%5u | Ucalc:%7.1f mV | Umeas:%5u mV | error:%7.2f %%\n",
                  raw, Ucalc, Umeas, error);
    delay(100);
}

