#include <Arduino.h>


#define LED_PIN 48     // one pin drives both LEDs
#define BLINK_MS 300

void setup()
{
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_PIN, HIGH);   // red is on  (current: pin → red → GND)
    delay(BLINK_MS);

    digitalWrite(LED_PIN, LOW);    // blue is on (current: 3V3 → blue → pin)
    delay(BLINK_MS);
}
