# Lesson 3.4 — The reverse: turning a number into a voltage (DAC)

[← Module 3](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8), ESP-IDF in C (PlatformIO, `framework = espidf`)

A DAC turns a code into a voltage: an R-2R ladder, the LSB, why the resolution isn't the accuracy. The ESP32-S3 (and
the STM32F411) have no DAC, so PWM through an RC low-pass stands in for one: duty × 3.3 V after the filter.

The homework's assignment, translated in full, is in the
[homework step's README](01-homework-buzzer-player-esp32/README.md#assignment).

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | lecture-pwm-rc-adc: [esp32](00-lecture-pwm-rc-adc-esp32/) | lecture | 20 kHz PWM on GPIO 18 through a 10 kΩ / 100 nF low-pass, read back by ADC1 on GPIO 1 every 20 ms; the duty follows a sawtooth, a sine and a triangle (0.63..2.60 V measured); a Python script plots the readings live |
| 01 | homework-buzzer-player: [esp32](01-homework-buzzer-player-esp32/) | homework | both tasks: a passive buzzer plays through LEDC on a fixed 50 ms `esp_timer` tick, with no task and no delay; the five example melodies one after another, while a second timer blinks an LED ([assignment](01-homework-buzzer-player-esp32/README.md#assignment)) |

## Working with the steps

Every ESP32 step is an ordinary PlatformIO project on ESP-IDF, written in C: `platformio.ini`, `src/main.c`, the two
`CMakeLists.txt` the PlatformIO wizard makes for ESP-IDF, and `sdkconfig.defaults` (the full `sdkconfig.<env>` is
generated at build time and not tracked).

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the PlatformIO buttons,
  or from the terminal: `pio run -d <step-folder>`.
- **Serial Monitor:** the UART USB-C port, 115200 baud (set in `platformio.ini`).
