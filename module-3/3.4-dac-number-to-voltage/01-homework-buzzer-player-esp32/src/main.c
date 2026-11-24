#include <stdbool.h>
#include <stdint.h>

#include "buzzer.h"
#include "esp_err.h"
#include "esp_timer.h"

#define TICK_MS 50          // the fixed tick: everything in the player happens on it
#define BEEP_TICKS 10       // 10 ticks = 500 ms on, 500 ms off
#define NOTE_A4 440         // Hz

// Called by esp_timer every TICK_MS. It runs in the esp_timer service (no task of ours, no delay),
// so it must return quickly: it only counts and switches the buzzer.
static void on_tick(void *arg)
{
    (void)arg;
    static uint32_t ticks = 0;
    static bool on = false;

    if (ticks % BEEP_TICKS == 0) {
        on = !on;
        if (on) {
            buzzer_tone(NOTE_A4);
        } else {
            buzzer_off();
        }
    }
    ticks++;
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
