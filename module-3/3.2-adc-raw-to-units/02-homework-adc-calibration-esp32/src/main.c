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
#define HEADER_EVERY 20              // repeat the table's header every 20 rows (2 s), so it stays on screen
#define CURVE_STEP   16              // the curve printed at start: every 16th code, 257 points

// The textbook formula: the code's share of the largest code, times the input range. It assumes the ADC is
// ideal: no offset, no gain error, the same on every chip.
static float raw_to_mv_manual(int raw)
{
    return raw * ATTEN_MAX_MV / ADC_MAX_CODE;
}

// How far the formula is from the calibrated value, in % of the calibrated value (+: the formula reads high)
static float error_percent(float mv_manual, int mv_cali)
{
    return (mv_manual - mv_cali) * 100.0f / mv_cali;
}

// The optional part: both conversions for the whole code range, once at start, as CSV lines for a plot. The
// error depends only on the code, so this shows the whole curve without turning the knob.
static void print_curve(adc_cali_handle_t cali)
{
    printf("CURVE,raw,u_manual_mv,u_cali_mv\n");
    for (int i = 0; i <= ADC_MAX_CODE / CURVE_STEP + 1; i++)
    {
        int raw = (i * CURVE_STEP > ADC_MAX_CODE) ? ADC_MAX_CODE : i * CURVE_STEP;  // the last point is 4095
        int mv_cali;

        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali, raw, &mv_cali));  // no conversion: only the curve
        printf("CURVE,%d,%.1f,%d\n", raw, raw_to_mv_manual(raw), mv_cali);
    }
}

// The settings the formula depends on, as the assignment suggests, then the table's header
static void print_header(void)
{
    printf("\nADC1 channel %d (GPIO 4), attenuation 12 dB (0..%.0f mV), 12 bits (0..%d), "
           "Vref ~1100 mV (corrected by the factory calibration)\n",
           POT_CHANNEL, ATTEN_MAX_MV, ADC_MAX_CODE);
    printf(" RAW   U_manual(mV)   U_cali(mV)   Error(%%)\n");  // %% prints one %
    printf("-------------------------------------------\n");
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
    print_curve(cali);

    for (unsigned row = 0; ; row++)
    {
        int raw;
        int mv_cali;
        float mv_manual;

        if (row % HEADER_EVERY == 0)
        {
            print_header();
        }
        ESP_ERROR_CHECK(adc_oneshot_read(adc, POT_CHANNEL, &raw));  // one conversion, waits for its result
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali, raw, &mv_cali));  // the same code through the curve
        mv_manual = raw_to_mv_manual(raw);
        if (mv_cali > 0)
        {
            printf("%4d   %12.1f   %10d   %8.2f\n", raw, mv_manual, mv_cali, error_percent(mv_manual, mv_cali));
        }
        else
        {
            printf("%4d   %12.1f   %10d   %8s\n", raw, mv_manual, mv_cali, "n/a");  // no % of zero
        }
        vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));  // gives the CPU to other tasks while waiting
    }
}
