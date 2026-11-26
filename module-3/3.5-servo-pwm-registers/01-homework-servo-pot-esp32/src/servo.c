#include "servo.h"

#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_GPIO 18 // the servo's signal wire (orange)
#define SERVO_MODE LEDC_LOW_SPEED_MODE
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_BITS LEDC_TIMER_14_BIT // the S3's maximum: 16384 steps in 20 ms, about 1.2 us a step
#define SERVO_STEPS (1u << 14)
#define SERVO_PWM_HZ 50 // a pulse every 20 ms
#define SERVO_PERIOD_US 20000u

void servo_init(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = SERVO_MODE,
        .duty_resolution = SERVO_BITS,
        .timer_num = SERVO_TIMER,
        .freq_hz = SERVO_PWM_HZ, // 80 MHz / (50 Hz x 16384) = a divider of about 97.7, inside 1..1023
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    const ledc_channel_config_t channel = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = SERVO_MODE,
        .channel = SERVO_CHANNEL,
        .timer_sel = SERVO_TIMER,
        .duty = 0, // no pulses until the first servo_set_pulse_us(): the servo stays limp
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

void servo_set_pulse_us(uint32_t pulse_us)
{
    // The pulse as a share of the period, in 14-bit steps, rounded: 1500 us -> 1229.
    const uint32_t duty = (pulse_us * SERVO_STEPS + SERVO_PERIOD_US / 2) / SERVO_PERIOD_US;
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty)); // the shadow register
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_MODE, SERVO_CHANNEL));    // takes effect at the next period
}
