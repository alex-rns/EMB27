# Lesson 3.5 — Precise position control (servo) and PWM set up through registers instead of libraries

[← Module 3](../)

**Board:** WeAct Black Pill (STM32F411CEU6), STM32CubeIDE, C on CMSIS registers

A hobby servo takes its angle from the width of a pulse repeated every 20 ms (about 1..2 ms, 1.5 ms the centre),
not from the average voltage. The lecture sets up the clock, SysTick, a timer's PWM and the ADC by writing their
registers directly, without HAL calls.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | lecture-servo: [stm32](00-lecture-servo-stm32/) | lecture | TIM4 on registers: a 1 µs tick, a 20 ms period, the pulse on PB6 (TIM4_CH1) drives an SG90; ADC1 on PA0 reads a potentiometer (8 readings averaged) and maps it to 500..2500 µs; 20.33 ms measured (the HSI runs 1.6 % slow) |

## Working with the steps

Every `-stm32` step is an STM32CubeIDE project: open it with File → Open Projects from File System, build with the
hammer, flash with Run → Run As → STM32 C/C++ Application and the ST-Link connected.
