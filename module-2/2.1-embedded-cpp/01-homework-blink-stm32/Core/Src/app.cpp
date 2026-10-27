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
	led.set(LedState::On);
	HAL_Delay(kBlinkIntervalMs);
	led.set(LedState::Off);
	HAL_Delay(kBlinkIntervalMs);
}
