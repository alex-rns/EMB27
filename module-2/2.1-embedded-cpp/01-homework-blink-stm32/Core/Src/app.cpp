/*
 * app.cpp
 *
 *  Created on: Oct 26, 2026
 *      Author: oleksandrbielodubrovskyi
 */

#include "app.h"
#include "main.h"

void app_setup() {

}

void app_loop() {
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	HAL_Delay(1000);
}
