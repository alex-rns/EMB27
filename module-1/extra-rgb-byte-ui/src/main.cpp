#include <Arduino.h>


#define LED_1_BIT_PIN 18
#define LED_2_BIT_PIN 17
#define LED_4_BIT_PIN 16
#define LED_8_BIT_PIN 15
#define LED_16_BIT_PIN 7
#define LED_32_BIT_PIN 6
#define LED_64_BIT_PIN 5
#define LED_128_BIT_PIN 4
#define BTN_CHANGE_PIN 9


void setup()
{
    Serial.begin(115200);

    pinMode(LED_1_BIT_PIN, OUTPUT);
    pinMode(LED_2_BIT_PIN, OUTPUT);
    pinMode(LED_4_BIT_PIN, OUTPUT);
    pinMode(LED_8_BIT_PIN, OUTPUT);
    pinMode(LED_16_BIT_PIN, OUTPUT);
    pinMode(LED_32_BIT_PIN, OUTPUT);
    pinMode(LED_64_BIT_PIN, OUTPUT);
    pinMode(LED_128_BIT_PIN, OUTPUT);
    pinMode(BTN_CHANGE_PIN, INPUT_PULLUP);
}

uint8_t x = 1;
uint8_t btn_prev_level = HIGH;
uint32_t last_read_time = 0;
uint32_t btn_press_count = 0;


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

    uint32_t now = millis();

    if (now - last_read_time >= 20) {
        last_read_time = now;
        uint8_t btn_level = digitalRead(BTN_CHANGE_PIN);
        if ( btn_prev_level == HIGH && btn_level == LOW) {
            btn_press_count = btn_press_count + 1;
            Serial.println(btn_press_count);
            x = x << 1;
            if (x == 0) { x = 1; }
            Serial.printf("press %u: x = %u\n", btn_press_count, x);
        }
        btn_prev_level = btn_level;
    };
    

}