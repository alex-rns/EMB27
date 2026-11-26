#include "servo.h"

#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_GPIO 18 // the servo's signal wire (orange)
#define SERVO_MODE LEDC_LOW_SPEED_MODE
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_BITS LEDC_TIMER_14_BIT // the S3's maximum: 16384 steps in 20 ms, about 1.2 us a step
#define SERVO_STEPS 16384u // 2 to the 14th
#define SERVO_PWM_HZ 50 // a pulse every 20 ms
#define SERVO_PERIOD_US 20000u
#define SERVO_MIN_US 500u  // 0 degrees: the lecture's ends, wider than the SG90's nominal 1000..2000 us
#define SERVO_MAX_US 2500u // SERVO_MAX_DEG

void servo_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = SERVO_MODE,
        .duty_resolution = SERVO_BITS,
        .timer_num = SERVO_TIMER,
        .freq_hz = SERVO_PWM_HZ, // 80 MHz / (50 Hz x 16384) = a divider of about 97.7, inside 1..1023
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channel = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = SERVO_MODE,
        .channel = SERVO_CHANNEL,
        .timer_sel = SERVO_TIMER,
        .duty = 0, // no pulses until the first servo_set_angle(): the servo stays limp
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

void servo_set_angle(float angle_deg)
{
    if (angle_deg > SERVO_MAX_DEG) {
        angle_deg = SERVO_MAX_DEG; // past the end the servo would push against its stop
    }
    // 0..180 deg -> 500..2500 us, then the pulse's share of the 20 ms period in 14-bit steps:
    // 90 deg -> 1500 us -> 1229 steps.
    float pulse_us = SERVO_MIN_US + angle_deg * (SERVO_MAX_US - SERVO_MIN_US) / SERVO_MAX_DEG;
    uint32_t duty = (uint32_t)(pulse_us * SERVO_STEPS / SERVO_PERIOD_US + 0.5f); // rounded
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty)); // the shadow register
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_MODE, SERVO_CHANNEL));    // takes effect at the next period
}
