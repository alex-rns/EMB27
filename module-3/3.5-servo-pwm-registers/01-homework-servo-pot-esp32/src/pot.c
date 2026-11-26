#include "pot.h"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#define POT_CHANNEL ADC_CHANNEL_0 // GPIO 1 on the ESP32-S3
#define POT_ATTEN ADC_ATTEN_DB_12 // an input range of about 0..3100 mV; the code reaches 4095 near 3150 mV
#define POT_BITS ADC_BITWIDTH_12  // codes 0..4095
#define POT_SAMPLES 16            // averaged: the reading jumps by a few codes on its own

static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;

void pot_init(void)
{
    const adc_oneshot_unit_init_cfg_t unit = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit, &s_adc));

    const adc_oneshot_chan_cfg_t channel = {
        .atten = POT_ATTEN,
        .bitwidth = POT_BITS,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc, POT_CHANNEL, &channel));

    // The factory calibration in eFuse turns a code into millivolts (the S3 uses the curve-fitting scheme).
    const adc_cali_curve_fitting_config_t cali = {
        .unit_id = ADC_UNIT_1,
        .chan = POT_CHANNEL,
        .atten = POT_ATTEN,
        .bitwidth = POT_BITS,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali, &s_cali));
}

int pot_read_mv(void)
{
    int sum = 0;
    for (int i = 0; i < POT_SAMPLES; i++) {
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(s_adc, POT_CHANNEL, &raw));
        sum += raw;
    }
    int mv;
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(s_cali, sum / POT_SAMPLES, &mv));
    return mv;
}
