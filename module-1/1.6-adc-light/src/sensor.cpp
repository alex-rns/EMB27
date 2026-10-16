#include "sensor.h"
#include "config.h"

namespace {
uint16_t samples[Config::kAverageWindow] = {};
uint32_t sum = 0;
size_t writeIndex  = 0;
size_t count = 0;
}  // namespace

namespace Sensor {
void begin() {
    analogReadResolution(12); // RAW code from 0 to 4095
    analogSetPinAttenuation(Config::kLdrGpio, ADC_11db);
    // ADC_11db extends the measurable range; near 3.3 V, ADC readings
    // can saturate/nonlinear. It does not make the ADC a precision voltmeter.
}

SensorReading sample() {
    SensorReading reading;
    reading.raw = static_cast<uint16_t>(analogRead(Config::kLdrGpio));
    // Separate ADC conversion, not a calibration of the exact RAW sample.
    reading.millivolts = analogReadMilliVolts(Config::kLdrGpio);

    if (count == Config::kAverageWindow) {
        sum -= samples[writeIndex];
    } else {
        ++count;
    }
    samples[writeIndex] = reading.raw;
    sum += reading.raw;
    writeIndex = (writeIndex + 1) % Config::kAverageWindow;
    reading.average = static_cast<uint16_t>(sum / count);
    return reading;
}
}  // namespace Sensor
