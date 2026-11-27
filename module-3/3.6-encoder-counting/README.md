# Lesson 3.6 — Counting a motor's turns or a wheel's position with an encoder

[← Module 3](../)

**Board:** WeAct Black Pill (STM32F411CEU6), STM32CubeIDE, HAL

An incremental encoder gives two square waves, A and B, a quarter cycle apart. The order of their edges gives the
direction, and their count gives the position. Speed is the count's change over time. A timer in encoder mode
decodes both lines in hardware, so the CPU only reads a counter now and then.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | lecture-encoder: [stm32](00-lecture-encoder-stm32/) | lecture | TIM3 in encoder mode TI12 (X4) on PA6/PA7 counts a KY-040's steps in hardware. Every 100 ms the code turns the count into position, direction, angle, RPM (raw and smoothed) and distance for the debugger's Live Expressions |

## Working with the steps

Every `-stm32` step is an STM32CubeIDE project: open it with File → Open Projects from File System, build with the
hammer, flash or debug with Run → Debug As → STM32 C/C++ Application and the ST-Link connected.
