#include <stddef.h>

#include "buzzer.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "melodies.h"
#include "player.h"

#define TICK_MS 50 // the fixed tick: everything in the player happens on it
#define LED_GPIO 6 // an LED through 220 Ohm: blinks while the melodies play
#define BLINK_MS 250

static const char *TAG = "player";

static size_t s_melody_index; // the melody to play next

// Called by esp_timer every TICK_MS. It runs in the esp_timer service (no task of ours, no delay),
// so it must return quickly: when a melody is over it starts the next one, then moves the player one tick on.
static void on_tick(void *arg)
{
    (void)arg;
    if (!player_is_playing()) {
        const melody_t *melody = melodies_get(s_melody_index);
        ESP_LOGI(TAG, "%s", melody->name);
        player_start(melody->notes, melody->count);
        s_melody_index = (s_melody_index + 1) % melodies_count();
    }
    player_tick();
}

// A second, independent job on its own esp_timer: the player never waits, so this keeps its rhythm.
static void on_blink(void *arg)
{
    (void)arg;
    static int level = 0;
    level = !level;
    gpio_set_level(LED_GPIO, level);
}

static void start_periodic(esp_timer_cb_t callback, const char *name, uint32_t period_ms)
{
    const esp_timer_create_args_t args = {
        .callback = callback,
        .name = name,
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, period_ms * 1000)); // the period is in microseconds
}

void app_main(void)
{
    buzzer_init();
    ESP_ERROR_CHECK(gpio_reset_pin(LED_GPIO));
    ESP_ERROR_CHECK(gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT));

    start_periodic(on_tick, "player_tick", TICK_MS);
    start_periodic(on_blink, "led_blink", BLINK_MS);
    ESP_LOGI(TAG, "started; app_main returns, the timers keep the music and the LED going");
    // app_main returns: nothing here waits
}
