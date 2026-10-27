/*
 * config.h
 *
 *  Created on: Oct 27, 2026
 *      Author: oleksandrbielodubrovskyi
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#include "main.h"

struct Config {
	static constexpr uint16_t kLedPin = GPIO_PIN_13;
	static constexpr uint32_t kBlinkIntervalMs = 1000;
	static GPIO_TypeDef* ledPort() { return GPIOC; }
};

#endif /* INC_CONFIG_H_ */
