#include <Arduino.h>


#define LED_1_BIT_PIN 18
#define LED_2_BIT_PIN 17
#define LED_4_BIT_PIN 16
#define LED_8_BIT_PIN 15
#define LED_16_BIT_PIN 7
#define LED_32_BIT_PIN 6
#define LED_64_BIT_PIN 5
#define LED_128_BIT_PIN 4
#define BTN_MOVE_PIN 9
#define BTN_TOGGLE_PIN 10


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

    pinMode(BTN_MOVE_PIN, INPUT_PULLUP);
    pinMode(BTN_TOGGLE_PIN, INPUT_PULLUP);
}

uint8_t cursor_position = 1;
uint8_t btn_prev_level = HIGH;
uint8_t btn_toggle_prev_level = HIGH;
uint32_t last_read_time = 0;
uint32_t btn_press_count = 0;
uint8_t channel_value = 0;




void loop()
{
    uint8_t display_byte = channel_value | cursor_position;

    digitalWrite(LED_1_BIT_PIN, (display_byte & 1) ? HIGH : LOW);
    digitalWrite(LED_2_BIT_PIN, (display_byte & 2) ? HIGH : LOW);
    digitalWrite(LED_4_BIT_PIN, (display_byte & 4) ? HIGH : LOW);
    digitalWrite(LED_8_BIT_PIN, (display_byte & 8) ? HIGH : LOW);
    digitalWrite(LED_16_BIT_PIN, (display_byte & 16) ? HIGH : LOW);
    digitalWrite(LED_32_BIT_PIN, (display_byte & 32) ? HIGH : LOW);
    digitalWrite(LED_64_BIT_PIN, (display_byte & 64) ? HIGH : LOW);
    digitalWrite(LED_128_BIT_PIN, (display_byte & 128) ? HIGH : LOW);

    uint32_t now = millis();
    

    if (now - last_read_time >= 20) {
        last_read_time = now;
        uint8_t btn_level = digitalRead(BTN_MOVE_PIN);
        uint8_t btn_toggle_level = digitalRead(BTN_TOGGLE_PIN);
        
        if ( btn_prev_level == HIGH && btn_level == LOW) {
            btn_press_count = btn_press_count + 1;
            cursor_position = cursor_position << 1;
            if (cursor_position == 0) { cursor_position = 1; }
            Serial.printf( "press %u: cursor_position = %u\n", btn_press_count, cursor_position);
        }
        btn_prev_level = btn_level;

        if ( btn_toggle_prev_level == HIGH && btn_toggle_level == LOW) {
            channel_value = channel_value ^ cursor_position;
            Serial.printf( "toggle: channel_value = %u\n", channel_value);
        }
        btn_toggle_prev_level = btn_toggle_level;
    };
    

}