#include <Arduino.h>


#define BLUE_LED_PIN 48
#define RED_LED_PIN 47
#define FLASH_MS 60
#define PAUSE_MS 150

void setup()
{
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(RED_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(RED_LED_PIN, LOW);
    delay(FLASH_MS);
    digitalWrite(RED_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(RED_LED_PIN, LOW);
    delay(PAUSE_MS);

    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(FLASH_MS);
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(PAUSE_MS);
}
