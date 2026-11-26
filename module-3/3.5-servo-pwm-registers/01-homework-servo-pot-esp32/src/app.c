#include "app.h"

#include "esp_log.h"
#include "pot.h"
#include "servo.h"

#define HYST_DEG 1.5f // above the noise left after 128 readings: 1.0 still let a still knob flicker about once a second

static const char *TAG = "servo_pot";

static float held_deg;      // the knob's angle after the hysteresis
static int logged_deg = -1; // the angle last logged; -1: nothing logged yet

// Hysteresis, as in 3.1: the angle moves only once the knob is more than HYST_DEG away from it, then follows the
// knob HYST_DEG behind, so a turn stays smooth and the noise of a still knob stays out.
static float filter_noise(float knob_deg)
{
    if (knob_deg > held_deg + HYST_DEG) {
        held_deg = knob_deg - HYST_DEG;
    } else if (knob_deg < held_deg - HYST_DEG) {
        held_deg = knob_deg + HYST_DEG;
    }
    return held_deg;
}

void app_init(void)
{
    servo_init();
    pot_init();
}

void app_run(void)
{
    // Point 1: 1:1, both counted from their leftmost position.
    float pot_deg = filter_noise(pot_read_deg());

    // Point 2: the pot turns about 300 deg, the servo 180; only the shared 0..180 is used, past it the servo stays
    // at 180.
    float servo_deg = pot_deg < SERVO_MAX_DEG ? pot_deg : SERVO_MAX_DEG;
    servo_set_angle(servo_deg);

    // Point 3: the angle from the leftmost position, in whole degrees, when it changes.
    int deg = (int)(servo_deg + 0.5f); // rounded
    if (deg != logged_deg) {
        ESP_LOGI(TAG, "angle %d deg from the leftmost position", deg);
        logged_deg = deg;
    }
}
