#include <math.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "pot.h"
#include "servo.h"

#define TICK_MS 20 // one servo period: a new angle for every pulse; reading the pot takes 0.85 ms of it

static const char *TAG = "servo_pot";

// Called by esp_timer every TICK_MS, in the esp_timer service (no task of ours, no delay).
static void on_tick(void *arg)
{
    (void)arg;
    static long s_logged_deg = -1; // nothing logged yet

    // 1:1, both from their leftmost position: the pot turns about 300 deg, the servo 180, so past 180 only the
    // pot moves on and the servo stays at 180.
    const float pot_deg = pot_read_deg();
    const float servo_deg = pot_deg < SERVO_MAX_DEG ? pot_deg : SERVO_MAX_DEG;
    servo_set_angle(servo_deg);

    const long deg = lroundf(servo_deg); // whole degrees in the log
    if (deg != s_logged_deg) { // only changes: a still knob keeps the console quiet
        ESP_LOGI(TAG, "angle %ld deg from the leftmost position", deg);
        s_logged_deg = deg;
    }
}

void app_main(void)
{
    servo_init();
    pot_init();

    const esp_timer_create_args_t args = {
        .callback = on_tick,
        .name = "tick",
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, TICK_MS * 1000)); // the period is in microseconds
}
