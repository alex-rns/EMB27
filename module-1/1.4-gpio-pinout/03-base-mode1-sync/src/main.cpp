#include <Arduino.h>

#define GREEN_LED_PIN 15
#define BLUE_LED_PIN 16
#define BTN_PIN 21

void setup()
{
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
}

void loop()
{
    int btnState = digitalRead(BTN_PIN);

    if (btnState == LOW)
    {
        digitalWrite(BLUE_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(200);
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);
        delay(200);
    }
}
