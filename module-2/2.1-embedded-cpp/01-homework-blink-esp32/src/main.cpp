#include <Arduino.h>

void setup() {
    pinMode(7, OUTPUT);       // the LED pin is an output
}

void loop() {
    digitalWrite(7, HIGH);    // turn the LED on
    delay(1000);              // wait for a second
    digitalWrite(7, LOW);     // turn the LED off
    delay(1000);              // wait for a second
}
