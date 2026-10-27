/*
 * led.h
 *
 *  Created on: Oct 27, 2026
 *      Author: oleksandrbielodubrovskyi
 */

#ifndef INC_LED_H_
#define INC_LED_H_

#include "main.h"
#include "config.h"

enum class LedState {
	On,
	Off
};

class Led {
	public:

	void init() {
		set(LedState::Off);
	}

	void set(LedState state) {
		HAL_GPIO_WritePin(Config::ledPort(), Config::kLedPin, state == LedState::On ? GPIO_PIN_RESET : GPIO_PIN_SET);
	}
};

#endif /* INC_LED_H_ */
