#include <Arduino.h>

// Keep defined for interrupts; comment out for polling.
#define USE_INTERRUPT_DEBOUNCE

// Selectable debounce example: ESP32-S3 + FX2 logic analyzer.
// GPIO4: push-button input, external 10 kOhm pull-up to 3.3V.
// Button connects GPIO4 to GND when pressed.
// Logic analyzer input channels D0/D1/D2/D3 are MONITOR ONLY.
constexpr uint8_t BUTTON_PIN = 4;       // Analyzer D0: physical button node
constexpr uint8_t RAW_PIN = 5;          // Analyzer D1: firmware sampled RAW
constexpr uint8_t CLEAN_PIN = 6;        // Analyzer D2: debounced state
constexpr uint8_t TEST_PIN = 7;         // Analyzer D3: periodic reference
constexpr uint32_t DEBOUNCE_MS = 30;

uint8_t stableState = HIGH;
uint32_t cleanPressCount = 0;

#ifdef USE_INTERRUPT_DEBOUNCE
// Shared with the ISR; the ESP32 spinlock protection
portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;
volatile uint8_t lastRaw = HIGH;
volatile uint32_t lastRawChangeMs = 0;
volatile uint32_t rawPressCount = 0;

void ARDUINO_ISR_ATTR onButtonChange() {
  // Keep the ISR short: record activity, with no delay or Serial calls.
  portENTER_CRITICAL_ISR(&buttonMux);
  const uint8_t raw = digitalRead(BUTTON_PIN);
  lastRawChangeMs = millis(); // Restart the quiet period on every interrupt.
  if (raw == LOW && lastRaw != LOW) {
    ++rawPressCount; // Includes bounce; fast edges can still be missed.
  }
  lastRaw = raw;
  portEXIT_CRITICAL_ISR(&buttonMux);
}
#else
// Polling mode: only loop() updates these values.
uint8_t lastRaw = HIGH;
uint32_t lastRawChangeMs = 0;
uint32_t rawPressCount = 0;
#endif

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT); // External 10 kOhm pull-up required.
  pinMode(RAW_PIN, OUTPUT);
  pinMode(CLEAN_PIN, OUTPUT);
  pinMode(TEST_PIN, OUTPUT);

  lastRaw = digitalRead(BUTTON_PIN);
  stableState = lastRaw;
  lastRawChangeMs = millis();
  digitalWrite(RAW_PIN, lastRaw);
  digitalWrite(CLEAN_PIN, stableState);
  digitalWrite(TEST_PIN, LOW);

#ifdef USE_INTERRUPT_DEBOUNCE
  // Track both presses and releases, including contact bounce.
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), onButtonChange, CHANGE);
#endif
}

void loop() {
#ifdef USE_INTERRUPT_DEBOUNCE
  // Copy ISR data together; read time afterward to avoid timestamp races.
  portENTER_CRITICAL(&buttonMux);
  const uint8_t raw = lastRaw;
  const uint32_t lastChangeMs = lastRawChangeMs;
  const uint32_t rawCount = rawPressCount;
  const uint32_t now = millis();
  portEXIT_CRITICAL(&buttonMux);
#else
  const uint32_t now = millis();
  const uint8_t raw = digitalRead(BUTTON_PIN);
  if (raw != lastRaw) {
    lastRawChangeMs = now;
    if (raw == LOW) {
      ++rawPressCount; // Includes bounce observed by polling.
    }
    lastRaw = raw;
  }
  const uint32_t lastChangeMs = lastRawChangeMs;
  const uint32_t rawCount = rawPressCount;
#endif
  digitalWrite(RAW_PIN, raw); // D1 mirrors the latest firmware sample.

  // Both modes accept a level after 30 ms with no observed button activity.
  if (raw != stableState &&
      static_cast<uint32_t>(now - lastChangeMs) >= DEBOUNCE_MS) {
    stableState = raw;
    if (stableState == LOW) {
      ++cleanPressCount;
    }
  }
  digitalWrite(CLEAN_PIN, stableState);

  // Nominal 400 ms period, 100 ms HIGH, 2.5 Hz, 25% duty cycle.
  // Software scheduling causes small edge-timing deviations (jitter).
  digitalWrite(TEST_PIN, (now % 400U) < 100U ? HIGH : LOW);

  // Slow logging is optional; printing can affect timing of D1 and D3.
  static uint32_t lastReportMs = 0;
  if (static_cast<uint32_t>(now - lastReportMs) >= 1000U) {
    lastReportMs = now;
    Serial.printf("RAW=%lu | DEBOUNCED=%lu\n",
                  static_cast<unsigned long>(rawCount),
                  static_cast<unsigned long>(cleanPressCount));
  }
}
