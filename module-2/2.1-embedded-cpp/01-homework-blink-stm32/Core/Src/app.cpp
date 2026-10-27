/*
 * app.cpp
 *
 *  Created on: Oct 26, 2026
 *      Author: oleksandrbielodubrovskyi
 */

#include "app.h"
#include "main.h"

enum class LedState {
	On,
	Off
};

constexpr uint16_t kLedPin = GPIO_PIN_13;
constexpr uint32_t kBlinkIntervalMs = 1000;

class Led {
	public:

	void init() {
		set(LedState::Off);
	}

	void set(LedState state) {
		HAL_GPIO_WritePin(GPIOC, kLedPin, state == LedState::On ? GPIO_PIN_RESET : GPIO_PIN_SET);
	}
};

Led led;

void app_setup() {
	led.init();

}

void app_loop() {
	static uint32_t lastToggleMs = 0;
	static LedState ledState = LedState::Off;

	const uint32_t nowMs = HAL_GetTick();

	if (nowMs - lastToggleMs >= kBlinkIntervalMs) {
		ledState = ledState == LedState::On ? LedState::Off : LedState::On;
		led.set(ledState);
		lastToggleMs = nowMs;
	}

}
