#include "driver/ledc.h" // LEDC: ledc_timer_config_t, ledc_timer_config()
#include "esp_err.h"     // ESP_ERROR_CHECK

#define MOTOR_GPIO 5 // to the 2N2222's base through a resistor
#define LED_GPIO 6   // to the LED through a resistor

#define PWM_MODE LEDC_LOW_SPEED_MODE // the S3 has only the low-speed group
#define PWM_TIMER LEDC_TIMER_0       // one timer for both channels, like TIM1 in the lecture
#define PWM_BITS LEDC_TIMER_14_BIT   // counter 0..16383; 10 Hz needs at least 13 bits (divider <= 1024)
#define PWM_FREQ_HZ 10               // slow enough to see by eye
#define MOTOR_CHANNEL LEDC_CHANNEL_0 // channel 0 of 8
#define LED_CHANNEL LEDC_CHANNEL_1

#define DUTY_MAX (1u << PWM_BITS) // 16384 = always high: the duty range is [0, 2^bits], like CCR = ARR + 1

void app_main(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_BITS,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK, // the driver picks the clock (APB 80 MHz here)
    };
    // The driver works out the divider itself: 80 MHz / (10 * 16384) = about 488
    ESP_ERROR_CHECK(ledc_timer_config(&timer)); // on an error: print it and restart

    const ledc_channel_config_t motor = {
        .gpio_num = MOTOR_GPIO,
        .speed_mode = PWM_MODE,
        .channel = MOTOR_CHANNEL,
        .timer_sel = PWM_TIMER, // the channel takes its frequency from this timer
        .duty = DUTY_MAX / 4,   // 25 %, like CCR1 = 250 of 1000 in the lecture
        .hpoint = 0,            // high from the period's start until the counter reaches duty
    };
    ESP_ERROR_CHECK(ledc_channel_config(&motor));

    const ledc_channel_config_t led = {
        .gpio_num = LED_GPIO,
        .speed_mode = PWM_MODE,
        .channel = LED_CHANNEL,
        .timer_sel = PWM_TIMER, // the same timer: the periods start together
        .duty = DUTY_MAX / 4,   // the same duty as the motor
        .hpoint = 0,
        .flags.output_invert = 1, // the pin inverted, like TIM1_CH1N: high 75 %
    };
    ESP_ERROR_CHECK(ledc_channel_config(&led));
}
