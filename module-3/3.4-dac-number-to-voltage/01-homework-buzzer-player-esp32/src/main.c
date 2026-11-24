#include <stddef.h>

#include "buzzer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "melodies.h"
#include "player.h"

#define TICK_MS 50 // the fixed tick: everything in the player happens on it

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

void app_main(void)
{
    buzzer_init();

    const esp_timer_create_args_t tick_args = {
        .callback = on_tick,
        .name = "player_tick",
    };
    esp_timer_handle_t tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(tick_timer, TICK_MS * 1000)); // the period is in microseconds
    // app_main returns: nothing here waits; the timer keeps calling on_tick
}
