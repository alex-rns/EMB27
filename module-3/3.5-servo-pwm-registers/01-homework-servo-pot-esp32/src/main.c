#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "pot.h"
#include "servo.h"

#define TICK_MS 100 // how often the potentiometer is read

static const char *TAG = "servo_pot";

// Called by esp_timer every TICK_MS, in the esp_timer service (no task of ours, no delay).
static void on_tick(void *arg)
{
    (void)arg;
    ESP_LOGI(TAG, "pot %d mV", pot_read_mv());
}

void app_main(void)
{
    servo_init();
    servo_set_angle(SERVO_MAX_DEG / 2); // the middle of the travel
    pot_init();

    const esp_timer_create_args_t args = {
        .callback = on_tick,
        .name = "tick",
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, TICK_MS * 1000)); // the period is in microseconds
}
