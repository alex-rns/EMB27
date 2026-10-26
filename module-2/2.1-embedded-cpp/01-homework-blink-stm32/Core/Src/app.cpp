/*
 * app.cpp
 *
 *  Created on: Oct 26, 2026
 *      Author: oleksandrbielodubrovskyi
 */

#include "app.h"
#include "main.h"

enum class LedState {On, Off};

constexpr uint16_t kLedPin = GPIO_PIN_13;
constexpr uint32_t kBlinkIntervalMs = 1000;

void app_setup() {

}

void app_loop() {
	HAL_GPIO_TogglePin(GPIOC, kLedPin);
	HAL_Delay(kBlinkIntervalMs);
}
