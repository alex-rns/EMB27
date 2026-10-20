#include <Arduino.h>


#define LED_1_BIT_PIN 18
#define LED_2_BIT_PIN 17
#define LED_4_BIT_PIN 16
#define LED_8_BIT_PIN 15
#define LED_16_BIT_PIN 7
#define LED_32_BIT_PIN 6
#define LED_64_BIT_PIN 5
#define LED_128_BIT_PIN 4


void setup()
{
    pinMode(LED_1_BIT_PIN, OUTPUT);
    pinMode(LED_2_BIT_PIN, OUTPUT);
    pinMode(LED_4_BIT_PIN, OUTPUT);
    pinMode(LED_8_BIT_PIN, OUTPUT);
    pinMode(LED_16_BIT_PIN, OUTPUT);
    pinMode(LED_32_BIT_PIN, OUTPUT);
    pinMode(LED_64_BIT_PIN, OUTPUT);
    pinMode(LED_128_BIT_PIN, OUTPUT);
}

uint8_t x = 160;

void loop()
{
    digitalWrite(LED_1_BIT_PIN, (x & 1) ? HIGH : LOW);
    digitalWrite(LED_2_BIT_PIN, (x & 2) ? HIGH : LOW);
    digitalWrite(LED_4_BIT_PIN, (x & 4) ? HIGH : LOW);
    digitalWrite(LED_8_BIT_PIN, (x & 8) ? HIGH : LOW);
    digitalWrite(LED_16_BIT_PIN, (x & 16) ? HIGH : LOW);
    digitalWrite(LED_32_BIT_PIN, (x & 32) ? HIGH : LOW);
    digitalWrite(LED_64_BIT_PIN, (x & 64) ? HIGH : LOW);
    digitalWrite(LED_128_BIT_PIN, (x & 128) ? HIGH : LOW);
}