/* app: the car's logic (the glue between the buttons, the sensors and the loads) and the superloop's time slots */
#include "app.h"
#include "main.h" /* the pin names from CubeMX: BTN_LOCK_Pin, BTN_LOCK_GPIO_Port, ... */
#include "button_fsm.h"
#include "crash_sensor.h"
#include "load_driver.h"
#include "pot_sensor.h"

/* The feedback, in ms: how long each beep and relay pulse lasts */
#define LOCK_BEEP_MS   80u  /* lock: one short beep */
#define UNLOCK_BEEP_MS 60u  /* unlock: two beeps, like a car; the gap between them is as long */
#define UNLOCK_BEEPS   2u
#define VALET_BEEP_MS  300u /* valet mode on or off: one long beep */
#define DIAG_PULSE_MS  200u /* HAZARD long press, the diagnostics: the relay and the buzzer together */
#define CRASH_PULSE_MS 150u /* crash: the relay and the buzzer together */

/* The flasher period follows the pot: from 200 ms (pot at 0) to 800 ms (pot at full) */
#define FLASH_PERIOD_MIN_MS 200u
#define FLASH_PERIOD_MAX_MS 800u

/* The four FSM buttons; the crash sensor is not here, it has its own interrupt */
typedef enum { IDX_LOCK, IDX_LEFT, IDX_RIGHT, IDX_HAZARD, N_FSM_BUTTONS } button_idx_t;

/* Where a button is wired: the FSM never sees this, it only gets "pressed or not" */
typedef struct {
	GPIO_TypeDef *port; /* GPIO port, e.g. GPIOB */
	uint16_t pin;       /* pin mask, e.g. GPIO_PIN_12 */
} button_pin_t;

/* const: the table never changes, so it stays in flash and takes no RAM. [IDX_...] = names the row it fills */
static const button_pin_t k_button_pins[N_FSM_BUTTONS] = {
	[IDX_LOCK]   = { BTN_LOCK_GPIO_Port, BTN_LOCK_Pin },
	[IDX_LEFT]   = { BTN_LEFT_GPIO_Port, BTN_LEFT_Pin },
	[IDX_RIGHT]  = { BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin },
	[IDX_HAZARD] = { BTN_HAZARD_GPIO_Port, BTN_HAZARD_Pin },
};
static button_fsm_t s_buttons[N_FSM_BUTTONS]; /* each button's FSM memory, in the same order */

/* The car's state: plain flags are enough here */
static bool s_locked = false;
static bool s_valet_mode = false;
static bool s_turn_l_on = false;
static bool s_turn_r_on = false;
static bool s_hazard_on = false;

/* HAL_GetTick() when each superloop slot ran last; static: they keep their value between app_loop() calls */
static uint32_t s_last_fsm_ms = 0;
static uint32_t s_last_pot_ms = 0;
static uint32_t s_last_blink_ms = 0;
static bool s_blink_phase = false; /* the flasher's half period: the turn LEDs on or off */

/* The glue: the only place where the buttons meet the loads. One small handler per button;
   each ignores BTN_EVENT_NONE, because no branch matches it */
static void handle_lock_button(button_event_t ev)
{
	if (ev == BTN_EVENT_SHORT_PRESS) {
		s_locked = !s_locked;
		load_relay_set(s_locked); /* the relay = the door lock */
		if (s_locked) {
			load_buzzer_chirp(LOCK_BEEP_MS);
		} else {
			load_buzzer_chirps(UNLOCK_BEEP_MS, UNLOCK_BEEPS);
		}
	} else if (ev == BTN_EVENT_LONG_PRESS) {
		s_valet_mode = !s_valet_mode;
		load_buzzer_chirp(VALET_BEEP_MS);
	}
}

/* left = true for LEFT, false for RIGHT. One side at a time: switching one on switches the other off */
static void handle_turn_button(button_event_t ev, bool left)
{
	if (ev == BTN_EVENT_SHORT_PRESS) {
		if (left) {
			s_turn_l_on = !s_turn_l_on;
			if (s_turn_l_on) {
				s_turn_r_on = false;
			}
		} else {
			s_turn_r_on = !s_turn_r_on;
			if (s_turn_r_on) {
				s_turn_l_on = false;
			}
		}
	}
	/* LONG = the lane change (3 blinks): challenge 1, nothing yet */
}

static void handle_hazard_button(button_event_t ev)
{
	if (ev == BTN_EVENT_SHORT_PRESS) {
		s_hazard_on = !s_hazard_on;
	} else if (ev == BTN_EVENT_LONG_PRESS) {
		load_relay_pulse(DIAG_PULSE_MS); /* diagnostics: the relay and the buzzer together */
		load_buzzer_chirp(DIAG_PULSE_MS);
	}
}

/* The crash is a safety event: it wins over the user's buttons */
static void handle_crash(void)
{
	s_hazard_on = true;
	load_relay_pulse(CRASH_PULSE_MS);
	load_buzzer_chirp(CRASH_PULSE_MS);
}

/* Read one button and advance its FSM. No pin setup here: CubeMX made the pins inputs with a pull-up,
   the button goes to GND, so pressed = LOW */
static button_event_t button_tick(button_idx_t idx, uint32_t now)
{
	const button_pin_t *p = &k_button_pins[idx];
	bool pressed = (HAL_GPIO_ReadPin(p->port, p->pin) == GPIO_PIN_RESET);
	return button_fsm_tick(&s_buttons[idx], pressed, now);
}

/* The state on the LEDs instead of the PDF's status log: the turn LEDs blink in the flasher's phase */
static void lamps_update(void)
{
	load_led_left_set(s_blink_phase && (s_hazard_on || s_turn_l_on)); /* the hazard wins; the turn flag stays and comes back */
	load_led_right_set(s_blink_phase && (s_hazard_on || s_turn_r_on));
	load_led_valet_set(s_valet_mode);
}

/* Each slot below runs its work when its period has passed and returns at once otherwise: nothing waits.
   The unsigned subtraction now - last survives the HAL_GetTick() wrap */

/* Every 10 ms: the buttons, then the LEDs, so a press shows at once */
static void buttons_slot(uint32_t now)
{
	if ((now - s_last_fsm_ms) >= BTN_FSM_TICK_MS) {
		s_last_fsm_ms = now;
		handle_lock_button(button_tick(IDX_LOCK, now));
		handle_turn_button(button_tick(IDX_LEFT, now), true);
		handle_turn_button(button_tick(IDX_RIGHT, now), false);
		handle_hazard_button(button_tick(IDX_HAZARD, now));
		lamps_update();
	}
}

/* Every 50 ms: the potentiometer */
static void pot_slot(uint32_t now)
{
	if ((now - s_last_pot_ms) >= POT_SAMPLE_MS) {
		s_last_pot_ms = now;
		pot_sensor_update();
	}
}

/* Every half flasher period: the LED phase and the click change */
static void flasher_slot(uint32_t now)
{
	/* the pot sets the period (integer map, no float); the largest product, 4095 * 600, fits in 32 bits */
	uint32_t blink_period_ms = FLASH_PERIOD_MIN_MS +
		((uint32_t)pot_sensor_get_raw() * (FLASH_PERIOD_MAX_MS - FLASH_PERIOD_MIN_MS)) / POT_RAW_MAX;
	if ((now - s_last_blink_ms) >= (blink_period_ms / 2u)) {
		s_last_blink_ms = now;
		bool blinking = s_hazard_on || s_turn_l_on || s_turn_r_on;
		s_blink_phase = blinking ? !s_blink_phase : false;
		load_buzzer_tick(s_blink_phase); /* tick on, tock off: each change clicks the passive buzzer */
	}
}

/* Every pass: the interrupt already caught the crash, only react here */
static void crash_slot(void)
{
	if (crash_sensor_take()) {
		handle_crash();
	}
}

void app_init(void)
{
	load_driver_init();
	for (int i = 0; i < N_FSM_BUTTONS; i++) {
		button_fsm_init(&s_buttons[i]); /* &: pass the struct's address, init fills it */
	}
}

void app_loop(void)
{
	uint32_t now = HAL_GetTick(); /* ms since start, one value for the whole pass */

	buttons_slot(now);
	pot_slot(now);
	flasher_slot(now);
	crash_slot();
	load_driver_update(); /* every pass: ends a beep or a relay pulse whose time is up */
}
