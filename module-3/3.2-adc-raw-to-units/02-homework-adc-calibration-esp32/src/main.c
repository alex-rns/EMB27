#include <stdio.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define POT_UNIT    ADC_UNIT_1
#define POT_CHANNEL ADC_CHANNEL_3    // GPIO 4: the potentiometer's middle pin
#define POT_ATTEN   ADC_ATTEN_DB_12  // the widest input range: up to about 3.1 V on the ESP32-S3
#define SAMPLE_MS   100              // one reading every ~100 ms, as the assignment asks
#define ADC_MAX_CODE 4095            // 12 bits: the largest code
#define ATTEN_MAX_MV 3100.0f         // the ESP32-S3's input range at 12 dB: 0..3100 mV (Espressif's ADC docs)

// The textbook formula: the code's share of the largest code, times the input range. It assumes the ADC is
// ideal: no offset, no gain error, the same on every chip.
static float raw_to_mv_manual(int raw)
{
    return raw * ATTEN_MAX_MV / ADC_MAX_CODE;
}

void app_main(void)
{
    adc_oneshot_unit_handle_t adc;
    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = POT_UNIT };
    adc_oneshot_chan_cfg_t chan_cfg = { .atten = POT_ATTEN, .bitwidth = ADC_BITWIDTH_12 };  // 12 bits: 0..4095
    adc_cali_handle_t cali;
    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = POT_UNIT, .chan = POT_CHANNEL, .atten = POT_ATTEN, .bitwidth = ADC_BITWIDTH_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc, POT_CHANNEL, &chan_cfg));
    // Reads the calibration data the factory wrote into this chip's eFuse and builds the chip's own curve
    ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali));

    while (1)
    {
        int raw;
        int mv_cali;

        ESP_ERROR_CHECK(adc_oneshot_read(adc, POT_CHANNEL, &raw));  // one conversion, waits for its result
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali, raw, &mv_cali));  // the same code through the curve
        printf("RAW %4d  U_manual %6.1f mV  U_cali %4d mV\n", raw, raw_to_mv_manual(raw), mv_cali);
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));  // gives the CPU to other tasks while waiting
    }
}
