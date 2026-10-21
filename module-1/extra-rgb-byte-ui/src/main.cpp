#include <Arduino.h>


#define LED_1_BIT_PIN 18
#define LED_2_BIT_PIN 17
#define LED_4_BIT_PIN 16
#define LED_8_BIT_PIN 15
#define LED_16_BIT_PIN 7
#define LED_32_BIT_PIN 6
#define LED_64_BIT_PIN 5
#define LED_128_BIT_PIN 4

#define LED_R_CHANNEL_PIN 38
#define LED_G_CHANNEL_PIN 39
#define LED_B_CHANNEL_PIN 40

#define BTN_MOVE_PIN 9
#define BTN_TOGGLE_PIN 10

#define BTN_R_CHANNEL_PIN 11
#define BTN_G_CHANNEL_PIN 12
#define BTN_B_CHANNEL_PIN 13

#define CHANNEL_R 0
#define CHANNEL_G 1
#define CHANNEL_B 2


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

    pinMode(LED_R_CHANNEL_PIN, OUTPUT);
    pinMode(LED_G_CHANNEL_PIN, OUTPUT);
    pinMode(LED_B_CHANNEL_PIN, OUTPUT);

    pinMode(BTN_MOVE_PIN, INPUT_PULLUP);
    pinMode(BTN_TOGGLE_PIN, INPUT_PULLUP);

    pinMode(BTN_R_CHANNEL_PIN, INPUT_PULLUP);
    pinMode(BTN_G_CHANNEL_PIN, INPUT_PULLUP);
    pinMode(BTN_B_CHANNEL_PIN, INPUT_PULLUP);
}

uint8_t cursor_position = 1;
uint8_t btn_move_prev_level = HIGH;
uint8_t btn_toggle_prev_level = HIGH;
uint32_t last_read_time = 0;
uint32_t btn_press_count = 0;
uint8_t channel_value = 0;

uint8_t chosen_channel = CHANNEL_R;
uint8_t btn_r_channel_prev_level = HIGH;
uint8_t btn_g_channel_prev_level = HIGH;
uint8_t btn_b_channel_prev_level = HIGH;

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

    digitalWrite(LED_R_CHANNEL_PIN, (chosen_channel == CHANNEL_R) ? HIGH : LOW);
    digitalWrite(LED_G_CHANNEL_PIN, (chosen_channel == CHANNEL_G) ? HIGH : LOW);
    digitalWrite(LED_B_CHANNEL_PIN, (chosen_channel == CHANNEL_B) ? HIGH : LOW);

    uint32_t now = millis();
    

    if (now - last_read_time >= 20) {
        last_read_time = now;
        uint8_t btn_move_level = digitalRead(BTN_MOVE_PIN);
        uint8_t btn_toggle_level = digitalRead(BTN_TOGGLE_PIN);

        uint8_t btn_r_channel_level = digitalRead(BTN_R_CHANNEL_PIN);
        uint8_t btn_g_channel_level = digitalRead(BTN_G_CHANNEL_PIN);
        uint8_t btn_b_channel_level = digitalRead(BTN_B_CHANNEL_PIN);

        if ( btn_move_prev_level == HIGH && btn_move_level == LOW) {
            btn_press_count = btn_press_count + 1;
            cursor_position = cursor_position << 1;
            if (cursor_position == 0) { cursor_position = 1; }
            Serial.printf( "press %u: cursor_position = %u\n", btn_press_count, cursor_position);
        }
        btn_move_prev_level = btn_move_level;

        if ( btn_toggle_prev_level == HIGH && btn_toggle_level == LOW) {
            channel_value = channel_value ^ cursor_position;
            Serial.printf( "toggle: channel_value = %u\n", channel_value);
        }
        btn_toggle_prev_level = btn_toggle_level;

        if ( btn_r_channel_prev_level == HIGH && btn_r_channel_level == LOW) {
            chosen_channel = CHANNEL_R;
            Serial.printf( "chosen_channel = %u\n", chosen_channel);
        }
        btn_r_channel_prev_level = btn_r_channel_level;

        if ( btn_g_channel_prev_level == HIGH && btn_g_channel_level == LOW) {
            chosen_channel = CHANNEL_G;
            Serial.printf( "chosen_channel = %u\n", chosen_channel);
        }
        btn_g_channel_prev_level = btn_g_channel_level;

        if ( btn_b_channel_prev_level == HIGH && btn_b_channel_level == LOW) {
            chosen_channel = CHANNEL_B;
            Serial.printf( "chosen_channel = %u\n", chosen_channel);
        }
        btn_b_channel_prev_level = btn_b_channel_level;
    }
    

}