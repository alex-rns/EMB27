#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h" /* CMSIS registers + HAL for the CubeMX files. */

/* Timer frequencies and the servo update period. */
#define SYSTICK_HZ         1000U
#define SERVO_TIMER_HZ     1000000U
#define SERVO_PWM_HZ       50U
#define SERVO_UPDATE_MS    (1000U / SERVO_PWM_HZ)

/* Pulse limits: calibrate them for the servo's full travel. */
#define SERVO_MIN_US       500U
#define SERVO_CENTER_US    1500U
#define SERVO_MAX_US       2500U
#define ADC_MAX_VALUE      4095U
#define ADC_AVERAGE_SAMPLES 8U

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
