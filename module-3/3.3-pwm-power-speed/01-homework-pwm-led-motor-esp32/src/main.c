#include "driver/ledc.h" // LEDC: ledc_timer_config_t, ledc_timer_config()
#include "esp_err.h"     // ESP_ERROR_CHECK

#define PWM_MODE LEDC_LOW_SPEED_MODE // the S3 has only the low-speed group
#define PWM_TIMER LEDC_TIMER_0       // one timer for both channels, like TIM1 in the lecture
#define PWM_BITS LEDC_TIMER_14_BIT   // counter 0..16383; 10 Hz needs at least 13 bits (divider <= 1024)
#define PWM_FREQ_HZ 10               // slow enough to see by eye

void app_main(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = PWM_MODE,
        .duty_resolution = PWM_BITS,
        .timer_num = PWM_TIMER,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK, // the driver picks the clock (APB 80 MHz here)
    };
    // The driver works out the divider itself: 80 MHz / (10 * 16384) = about 488. No channel yet, so no pin changes
    ESP_ERROR_CHECK(ledc_timer_config(&timer)); // on an error: print it and restart
}
