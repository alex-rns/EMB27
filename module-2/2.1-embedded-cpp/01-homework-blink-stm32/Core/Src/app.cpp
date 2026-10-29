/*
 * app.cpp
 *
 *  Created on: Oct 26, 2026
 *      Author: oleksandrbielodubrovskyi
 */

#include "app.h"
#include "main.h"
#include "config.h"
#include "led.h"

Led led;

void app_setup() {
	led.init();

}

void app_loop() {
	static uint32_t lastToggleMs = 0;
	static LedState ledState = LedState::Off;

	const uint32_t nowMs = HAL_GetTick();

	if (nowMs - lastToggleMs >= Config::kBlinkIntervalMs) {
		ledState = ledState == LedState::On ? LedState::Off : LedState::On;
		led.set(ledState);
		lastToggleMs = nowMs;
	}

}
