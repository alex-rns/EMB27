#include "buzzer.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "notes.h"
#include "player.h"

#define TICK_MS 50 // the fixed tick: everything in the player happens on it
#define QUARTER 8  // ticks: 8 x 50 ms = 400 ms
#define HALF 16

#define COUNT_OF(a) (sizeof(a) / sizeof((a)[0]))

// Twinkle Twinkle Little Star, the assignment's first line: C C G G | A A G
static const note_t s_twinkle[] = {
    {NOTE_C4, QUARTER}, {NOTE_C4, QUARTER}, {NOTE_G4, QUARTER}, {NOTE_G4, QUARTER},
    {NOTE_A4, QUARTER}, {NOTE_A4, QUARTER}, {NOTE_G4, HALF},    {NOTE_REST, HALF},
};

// Called by esp_timer every TICK_MS. It runs in the esp_timer service (no task of ours, no delay),
// so it must return quickly: it only moves the player one tick on.
static void on_tick(void *arg)
{
    (void)arg;
    if (!player_is_playing()) {
        player_start(s_twinkle, COUNT_OF(s_twinkle)); // over again
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
