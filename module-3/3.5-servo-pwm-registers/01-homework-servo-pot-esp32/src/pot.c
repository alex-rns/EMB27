#include "pot.h"

#include <stddef.h>

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

#define POT_CHANNEL ADC_CHANNEL_0 // GPIO 1 on the ESP32-S3
#define POT_ATTEN ADC_ATTEN_DB_12 // an input range of about 0..3100 mV; the code reaches 4095 near 3150 mV
#define POT_BITS ADC_BITWIDTH_12  // codes 0..4095
#define POT_SAMPLES 16            // averaged on every read: 0.85 ms (128 took 6.8 ms, measured)
#define POT_SMA_LEN 8             // the last 8 reads averaged again: 8 ticks of 20 ms = 160 ms
#define POT_FULL_MV 3300.0f       // the 3V3 rail at the right end (the ADC itself tops out near 3155 mV)
#define POT_TRAVEL_DEG 300.0f     // measured: 1990 mV at a half turn, 180 x 3300 / 1990 = about 298 deg
#define POT_BACKLASH_DEG 1.0f     // the output moves only when the knob gets this far from it

static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;
static int s_window[POT_SMA_LEN]; // the last reads in mV, the oldest replaced first
static int s_window_sum;
static size_t s_window_pos;
static float s_held_deg; // the angle pot_read_deg() last reported

static int pot_read_mv(void)
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

    // Start the average from a real reading, or at power-up the servo would swing towards 0 deg and back.
    const int mv = pot_read_mv();
    for (size_t i = 0; i < POT_SMA_LEN; i++) {
        s_window[i] = mv;
    }
    s_window_sum = mv * POT_SMA_LEN;
    s_held_deg = mv * POT_TRAVEL_DEG / POT_FULL_MV;
}

float pot_read_deg(void)
{
    // A moving average of the last POT_SMA_LEN reads: the oldest leaves the sum, the newest joins it.
    const int mv = pot_read_mv();
    s_window_sum += mv - s_window[s_window_pos];
    s_window[s_window_pos] = mv;
    s_window_pos = (s_window_pos + 1) % POT_SMA_LEN;

    // A linear pot: the angle is the voltage's share of the full 3V3. 1990 mV -> 181 deg.
    const float deg = (float)s_window_sum / POT_SMA_LEN * POT_TRAVEL_DEG / POT_FULL_MV;

    // Backlash: the output is dragged along only once the knob is more than POT_BACKLASH_DEG away, so it follows a
    // turn smoothly and ignores the noise of a still knob; it stays up to that much behind.
    if (deg > s_held_deg + POT_BACKLASH_DEG) {
        s_held_deg = deg - POT_BACKLASH_DEG;
    } else if (deg < s_held_deg - POT_BACKLASH_DEG) {
        s_held_deg = deg + POT_BACKLASH_DEG;
    }
    return s_held_deg;
}
