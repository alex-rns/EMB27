#include <Arduino.h>

#define GREEN_LED_PIN 15
#define BLUE_LED_PIN 16
#define BTN_PIN 21
#define BOOT_BTN 0

volatile int mode = 1;

void IRAM_ATTR onButton()
{
    mode = 1;
}

void IRAM_ATTR onBoot()
{
    mode = 2;
}

void setup()
{
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BTN_PIN, INPUT_PULLUP);
    pinMode(BOOT_BTN, INPUT);

    attachInterrupt(digitalPinToInterrupt(BTN_PIN), onButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BOOT_BTN), onBoot, FALLING);
}

void loop()
{
    if (mode == 1)
    {
        digitalWrite(BLUE_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(200);
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, LOW);
        delay(200);
    }
    else if (mode == 2)
    {
        digitalWrite(BLUE_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        delay(1000);
        digitalWrite(BLUE_LED_PIN, LOW);
        digitalWrite(GREEN_LED_PIN, HIGH);
        delay(1000);
    }
}
