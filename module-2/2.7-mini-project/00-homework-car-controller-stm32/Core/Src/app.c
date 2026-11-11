/* app: the car's logic (the glue between the buttons, the sensors and the loads) and the superloop's time slots */
#include "app.h"
#include "main.h" /* the pin names from CubeMX: BTN_LOCK_Pin, LED_Pin, ... */
#include "button_fsm.h"
#include "load_driver.h"
#include "pot_sensor.h"

/* The four FSM buttons; the crash sensor is not here, it has its own interrupt */
enum { IDX_LOCK, IDX_LEFT, IDX_RIGHT, IDX_HAZARD, N_FSM_BUTTONS };
static button_fsm_t s_buttons[N_FSM_BUTTONS];

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
			load_buzzer_chirp(80u); /* lock: one beep */
		} else {
			load_buzzer_chirps(60u, 2u); /* unlock: two beeps, like a car */
		}
	} else if (ev == BTN_EVENT_LONG_PRESS) {
		s_valet_mode = !s_valet_mode;
		load_buzzer_chirp(300u);
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
		load_relay_pulse(200u); /* diagnostics: the relay and the buzzer together for 200 ms */
		load_buzzer_chirp(200u);
	}
}

/* The crash is a safety event: it wins over the user's buttons */
static void handle_crash(void)
{
	s_hazard_on = true;
	load_relay_pulse(150u);
	load_buzzer_chirp(150u);
}

void app_init(void)
{
	load_driver_init();
	button_fsm_init(&s_buttons[IDX_LOCK], BTN_LOCK_GPIO_Port, BTN_LOCK_Pin); /* &: pass the struct's address, init fills it */
	button_fsm_init(&s_buttons[IDX_LEFT], BTN_LEFT_GPIO_Port, BTN_LEFT_Pin);
	button_fsm_init(&s_buttons[IDX_RIGHT], BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin);
	button_fsm_init(&s_buttons[IDX_HAZARD], BTN_HAZARD_GPIO_Port, BTN_HAZARD_Pin);
}

void app_loop(void)
{
	uint32_t now = HAL_GetTick(); /* ms since start; each slot below runs when its period has passed, nothing waits */

	if (now - s_last_fsm_ms >= BTN_FSM_TICK_MS) { /* every 10 ms: the buttons; unsigned subtraction survives the wrap */
		s_last_fsm_ms = now;
		handle_lock_button(button_fsm_tick(&s_buttons[IDX_LOCK], now));
		handle_turn_button(button_fsm_tick(&s_buttons[IDX_LEFT], now), true);
		handle_turn_button(button_fsm_tick(&s_buttons[IDX_RIGHT], now), false);
		handle_hazard_button(button_fsm_tick(&s_buttons[IDX_HAZARD], now));

		/* The state on the LEDs instead of the PDF's status log: the turn LEDs blink in the flasher's phase */
		load_led_left_set(s_blink_phase && (s_hazard_on || s_turn_l_on)); /* the hazard wins; the turn flag stays and comes back */
		load_led_right_set(s_blink_phase && (s_hazard_on || s_turn_r_on));
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, s_valet_mode ? GPIO_PIN_RESET : GPIO_PIN_SET); /* valet; lit on LOW */
	}

	if (now - s_last_pot_ms >= POT_SAMPLE_MS) { /* every 50 ms: the potentiometer */
		s_last_pot_ms = now;
		pot_sensor_update();
	}

	/* The flasher: the pot sets the period, 200..800 ms (integer map, no float); 4095 * 600 fits in 32 bits */
	uint32_t blink_period_ms = 200u + ((uint32_t)pot_sensor_get_raw() * 600u) / 4095u;
	if (now - s_last_blink_ms >= blink_period_ms / 2u) { /* every half period the LEDs and the click change */
		s_last_blink_ms = now;
		bool blinking = s_hazard_on || s_turn_l_on || s_turn_r_on;
		s_blink_phase = blinking ? !s_blink_phase : false;
		load_buzzer_tick(s_blink_phase); /* tick on, tock off: each change clicks the passive buzzer */
	}

	if (crash_sensor_take()) { /* every pass: the interrupt already caught it, only react here */
		handle_crash();
	}

	load_driver_update(); /* every pass: ends a beep or a relay pulse whose time is up */
}
