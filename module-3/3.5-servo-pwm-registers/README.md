# Lesson 3.5 — Precise position control (servo) and PWM set up through registers instead of libraries

[← Module 3](../)

**Boards:** the lecture on the WeAct Black Pill (STM32F411CEU6, STM32CubeIDE, C on CMSIS registers), the homework on
the ESP32-S3 DevKitC-1 clone (N16R8, ESP-IDF in C, PlatformIO)

A hobby servo takes its angle from the width of a pulse repeated every 20 ms (about 1..2 ms, 1.5 ms the centre),
not from the average voltage. The lecture sets up the clock, SysTick, a timer's PWM and the ADC by writing their
registers directly, without HAL calls.

The homework's assignment, translated in full, is in the
[homework step's README](01-homework-servo-pot-esp32/README.md#assignment).

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | lecture-servo: [stm32](00-lecture-servo-stm32/) | lecture | TIM4 on registers: a 1 µs tick, a 20 ms period, the pulse on PB6 (TIM4_CH1) drives an SG90; ADC1 on PA0 reads a potentiometer (8 readings averaged) and maps it to 500..2500 µs; 20.33 ms measured (the HSI runs 1.6 % slow) |
| 01 | homework-servo-pot: [esp32](01-homework-servo-pot-esp32/) | homework | a potentiometer on GPIO 1 sets an SG90 on GPIO 18 (LEDC, 50 Hz, 14 bits) 1:1 from the leftmost position: of the knob's ~300° only the servo's 180° are used; the angle is logged when it changes; one loop with a 20 ms delay, as in the lecture; 128 readings averaged and a 1.5° hysteresis keep the servo still under a still knob ([assignment](01-homework-servo-pot-esp32/README.md#assignment)) |

## Working with the steps

Every `-stm32` step is an STM32CubeIDE project: open it with File → Open Projects from File System, build with the
hammer, flash with Run → Run As → STM32 C/C++ Application and the ST-Link connected.

Every `-esp32` step is an ordinary PlatformIO project on ESP-IDF, written in C: build and upload with
`pio run -d <step-folder>` (add `-t upload`) or the PlatformIO buttons; the Serial Monitor is the UART USB-C port at
115 200 baud.
