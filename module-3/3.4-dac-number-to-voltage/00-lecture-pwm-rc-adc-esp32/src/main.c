/*
 * ESP32-S3: PWM -> RC low-pass -> ADC -> Serial Plotter.
 *
 * GPIO18 -- 10 kOhm --+-- GPIO1 (ADC1_CH0)
 *                     |
 *                   22 uF
 *                     |
 *                    GND
 *
 * Sawtooth -> sine -> triangle, 3 periods of each shape.
 * Prints the voltage in volts, 115200 baud.
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "driver/ledc.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define PWM_GPIO             18
#define PWM_FREQ_HZ          20000
#define PWM_DUTY_STEPS       1024    // 10 bits
#define ADC_CHANNEL          ADC_CHANNEL_0  // GPIO1 on the ESP32-S3
#define SAMPLE_PERIOD_MS     20
#define SIGNAL_PERIOD_MS     4000    // One period of the signal: 4 seconds
#define PERIODS_PER_WAVE     3
#define ADC_SAMPLES          16      // Averaging to reduce the noise
#define SAMPLES_PER_PERIOD   (SIGNAL_PERIOD_MS / SAMPLE_PERIOD_MS)

enum { WAVE_SAW, WAVE_SINE, WAVE_TRIANGLE, WAVE_COUNT };

static adc_oneshot_unit_handle_t adc;
static adc_cali_handle_t calibration;

static void init_pwm(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = PWM_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channel = {
        .gpio_num = PWM_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

static void init_adc(void)
{
    adc_oneshot_unit_init_cfg_t unit = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit, &adc));

    adc_oneshot_chan_cfg_t channel = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, ADC_CHANNEL, &channel));

    // On the ESP32-S3 use the Curve Fitting calibration.
    adc_cali_curve_fitting_config_t config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&config, &calibration));
}

// phase goes from 0 to 1 over one period. The result: from 0 to 1.
static float wave_value(int waveform, float phase)
{
    switch (waveform) {
    case WAVE_SINE:
        return 0.5f - 0.5f * cosf(6.2831853f * phase);
    case WAVE_TRIANGLE:
        return 1.0f - fabsf(2.0f * phase - 1.0f);
    default:
        return phase;  // Sawtooth: a smooth rise and a sharp drop
    }
}

static int read_voltage_mv(void)
{
    int sum = 0;
    for (int i = 0; i < ADC_SAMPLES; i++) {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc, ADC_CHANNEL, &raw));
        sum += raw;
    }

    int voltage_mv;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(
        calibration, sum / ADC_SAMPLES, &voltage_mv));
    return voltage_mv;
}

void app_main(void)
{
    init_pwm();
    init_adc();
    setvbuf(stdout, NULL, _IONBF, 0);

    int waveform = WAVE_SAW;
    int sample = 0;
    TickType_t last_wakeup = xTaskGetTickCount();

    while (1) {
        float phase = (float)(sample % SAMPLES_PER_PERIOD) / SAMPLES_PER_PERIOD;
        float value = wave_value(waveform, phase);

        // Duty 20..80 %: the voltage after the RC is about 0.66..2.64 V.
        uint32_t duty = (uint32_t)((0.2f + 0.6f * value) * PWM_DUTY_STEPS);
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

        // Let the RC respond, then measure the voltage.
        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(SAMPLE_PERIOD_MS));
        printf("voltage:%.3f\n", read_voltage_mv() / 1000.0);

        if (++sample >= SAMPLES_PER_PERIOD * PERIODS_PER_WAVE) {
            sample = 0;
            waveform = (waveform + 1) % WAVE_COUNT;
        }
    }
}
