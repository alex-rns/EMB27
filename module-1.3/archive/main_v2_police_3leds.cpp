#include <Arduino.h>


#define BLUE_LED_PIN 48
#define RED_LED_PIN 47
#define WHITE_LED_PIN 21

#define FLASH_MIN_MS 15
#define FLASH_MAX_MS 100
#define FLASH_STEP_MS 10

int flashMs = FLASH_MAX_MS;
int stepMs = -FLASH_STEP_MS;

void setup()
{
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(WHITE_LED_PIN, OUTPUT);
}

void loop()
{
    int pauseMs = flashMs * 5 / 2;

    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(WHITE_LED_PIN, HIGH);
    delay(flashMs);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(WHITE_LED_PIN, LOW);
    delay(flashMs);
    digitalWrite(BLUE_LED_PIN, HIGH);
    digitalWrite(WHITE_LED_PIN, HIGH);
    delay(flashMs);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(WHITE_LED_PIN, LOW);
    delay(pauseMs);

    digitalWrite(WHITE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    delay(flashMs);
    digitalWrite(WHITE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    delay(flashMs);
    digitalWrite(WHITE_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    delay(flashMs);
    digitalWrite(WHITE_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    delay(pauseMs);

    flashMs += stepMs;
    if (flashMs <= FLASH_MIN_MS || flashMs >= FLASH_MAX_MS) {
        stepMs = -stepMs;
    }
}
