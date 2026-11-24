#include "buzzer.h"

#include "driver/ledc.h"
#include "esp_err.h"

#define BUZZER_GPIO 15 // to the buzzer module's S pin through 220 Ohm
#define BUZZER_MODE LEDC_LOW_SPEED_MODE
#define BUZZER_TIMER LEDC_TIMER_0
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_BITS LEDC_TIMER_10_BIT // 80 MHz / 1024: tones from about 76 Hz to 78 kHz
#define BUZZER_DUTY_HALF (1u << (BUZZER_BITS - 1)) // 512 of 1024 = 50 %: the loudest square wave
#define BUZZER_START_HZ 440

void buzzer_init(void)
{
    const ledc_timer_config_t timer = {
        .speed_mode = BUZZER_MODE,
        .duty_resolution = BUZZER_BITS,
        .timer_num = BUZZER_TIMER,
        .freq_hz = BUZZER_START_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    const ledc_channel_config_t channel = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = BUZZER_MODE,
        .channel = BUZZER_CHANNEL,
        .timer_sel = BUZZER_TIMER,
        .duty = 0, // silent until the first tone
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

void buzzer_tone(uint32_t freq_hz)
{
    ESP_ERROR_CHECK(ledc_set_freq(BUZZER_MODE, BUZZER_TIMER, freq_hz)); // the timer's frequency = the note
    ESP_ERROR_CHECK(ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, BUZZER_DUTY_HALF));
    ESP_ERROR_CHECK(ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL));
}

void buzzer_off(void)
{
    ESP_ERROR_CHECK(ledc_set_duty(BUZZER_MODE, BUZZER_CHANNEL, 0));
    ESP_ERROR_CHECK(ledc_update_duty(BUZZER_MODE, BUZZER_CHANNEL));
}
