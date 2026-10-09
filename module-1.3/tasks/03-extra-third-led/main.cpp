#include <Arduino.h>


#define BLUE_LED_PIN 48
#define RED_LED_PIN 47
#define WHITE_LED_PIN 21
#define FLASH_MS 60
#define PAUSE_MS 150

void setup()
{
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(WHITE_LED_PIN, OUTPUT);
}

void loop()
{
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(WHITE_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(WHITE_LED_PIN, LOW);
    delay(FLASH_MS);
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(WHITE_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(WHITE_LED_PIN, LOW);
    delay(PAUSE_MS);

    digitalWrite(WHITE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(WHITE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    delay(FLASH_MS);
    digitalWrite(WHITE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    delay(FLASH_MS);
    digitalWrite(WHITE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    delay(PAUSE_MS);
}
