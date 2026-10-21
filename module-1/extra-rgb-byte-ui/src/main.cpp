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

#define BTN_TO_RGB_PIN 14

#define CHANNEL_R 0
#define CHANNEL_G 1
#define CHANNEL_B 2

#define RGB_R_PIN 21
#define RGB_G_PIN 41
#define RGB_B_PIN 42

#define RGB_R_LEDC_CHANNEL 0
#define RGB_G_LEDC_CHANNEL 1
#define RGB_B_LEDC_CHANNEL 2
#define RGB_PWM_FREQ 5000
#define RGB_PWM_BITS 8

#define CURSOR_BLINK_MS 250


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

    pinMode(BTN_TO_RGB_PIN, INPUT_PULLUP);

    pinMode(BTN_MOVE_PIN, INPUT_PULLUP);
    pinMode(BTN_TOGGLE_PIN, INPUT_PULLUP);

    pinMode(BTN_R_CHANNEL_PIN, INPUT_PULLUP);
    pinMode(BTN_G_CHANNEL_PIN, INPUT_PULLUP);
    pinMode(BTN_B_CHANNEL_PIN, INPUT_PULLUP);

    ledcSetup(RGB_R_LEDC_CHANNEL, RGB_PWM_FREQ, RGB_PWM_BITS);
    ledcSetup(RGB_G_LEDC_CHANNEL, RGB_PWM_FREQ, RGB_PWM_BITS);
    ledcSetup(RGB_B_LEDC_CHANNEL, RGB_PWM_FREQ, RGB_PWM_BITS);
    ledcAttachPin(RGB_R_PIN, RGB_R_LEDC_CHANNEL);
    ledcAttachPin(RGB_G_PIN, RGB_G_LEDC_CHANNEL);
    ledcAttachPin(RGB_B_PIN, RGB_B_LEDC_CHANNEL);
}

uint8_t cursor_position = 1;
uint8_t btn_move_prev_level = HIGH;
uint8_t btn_toggle_prev_level = HIGH;
uint32_t last_read_time = 0;
uint32_t btn_press_count = 0;
uint8_t channel_r_value = 0;
uint8_t channel_g_value = 0;
uint8_t channel_b_value = 0;

uint8_t chosen_channel = CHANNEL_R;
uint8_t btn_r_channel_prev_level = HIGH;
uint8_t btn_g_channel_prev_level = HIGH;
uint8_t btn_b_channel_prev_level = HIGH;

uint8_t btn_to_rgb_prev_level = HIGH;

void loop()
{
    uint8_t display_byte;
    if (chosen_channel == CHANNEL_R) {
        display_byte = channel_r_value;
    } else if (chosen_channel == CHANNEL_G) {
        display_byte = channel_g_value;
    } else {
        display_byte = channel_b_value;
    }

    // BLINK
    if ((millis() / CURSOR_BLINK_MS) % 2 == 1) {
        display_byte = display_byte | cursor_position;
    } else {
        display_byte = display_byte & ~cursor_position;
    }

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

        uint8_t btn_to_rgb_level = digitalRead(BTN_TO_RGB_PIN);

        // BTN #1
        if ( btn_move_prev_level == HIGH && btn_move_level == LOW) {
            btn_press_count = btn_press_count + 1;
            cursor_position = cursor_position << 1;
            if (cursor_position == 0) { cursor_position = 1; }
            Serial.printf( "press %u: cursor_position = %u\n", btn_press_count, cursor_position);
        }
        btn_move_prev_level = btn_move_level;

        // BTN #2
        if ( btn_toggle_prev_level == HIGH && btn_toggle_level == LOW) {
            if (chosen_channel == CHANNEL_R) {
                channel_r_value = channel_r_value ^ cursor_position;
            } else if (chosen_channel == CHANNEL_G) {
                channel_g_value = channel_g_value ^ cursor_position;
            } else {
                channel_b_value = channel_b_value ^ cursor_position;
            }
            Serial.printf( "toggle: R = %u, G = %u, B = %u\n", channel_r_value, channel_g_value, channel_b_value);
        }
        btn_toggle_prev_level = btn_toggle_level;

        // BTN #3
        if ( btn_r_channel_prev_level == HIGH && btn_r_channel_level == LOW) {
            chosen_channel = CHANNEL_R;
            cursor_position = 1;
            Serial.printf( "chosen_channel = %u\n", chosen_channel);
        }
        btn_r_channel_prev_level = btn_r_channel_level;

        // BTN #4
        if ( btn_g_channel_prev_level == HIGH && btn_g_channel_level == LOW) {
            chosen_channel = CHANNEL_G;
            cursor_position = 1;
            Serial.printf( "chosen_channel = %u\n", chosen_channel);
        }
        btn_g_channel_prev_level = btn_g_channel_level;

        // BTN #5
        if ( btn_b_channel_prev_level == HIGH && btn_b_channel_level == LOW) {
            chosen_channel = CHANNEL_B;
            cursor_position = 1;
            Serial.printf( "chosen_channel = %u\n", chosen_channel);
        }
        btn_b_channel_prev_level = btn_b_channel_level;

        // BTN #6
        if ( btn_to_rgb_prev_level == HIGH && btn_to_rgb_level == LOW) {
            ledcWrite(RGB_R_LEDC_CHANNEL, channel_r_value);
            ledcWrite(RGB_G_LEDC_CHANNEL, channel_g_value);
            ledcWrite(RGB_B_LEDC_CHANNEL, channel_b_value);
            Serial.printf( "rgb: R = %u, G = %u, B = %u\n", channel_r_value, channel_g_value, channel_b_value);
        }
        btn_to_rgb_prev_level = btn_to_rgb_level;
    }
}
