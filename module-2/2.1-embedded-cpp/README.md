# Lesson 2.1 — C++ for MCUs: resource limits and specifics. Homework: Embedded C++ blink

[← Module 2](../)

**Boards:** ESP32-S3 DevKitC-1 clone (N16R8), Arduino core 2.0.17 (PlatformIO) · WeAct Black Pill (STM32F411CEU6),
STM32CubeIDE

Embedded C++ on the ESP32: `enum class`, `constexpr`, a `Led` class, a non-blocking superloop on `millis()`,
the loop's own timing, and a button on an interrupt.

The assignment, translated in full, is in the [homework step's README](01-homework-blink-esp32/README.md#assignment).

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [practice-blink-esp32](00-practice-blink-esp32/) | practice | the homework tasks done once before the lecture |
| 01 | homework-blink: [esp32](01-homework-blink-esp32/) · [stm32](01-homework-blink-stm32/) | homework | the homework: the assignment, how each point is done, one commit per change; the STM32 version does the required tasks 1 and 2 |

## Working with the steps

Every `-esp32` step is an ordinary PlatformIO project: `platformio.ini`, `src/main.cpp`, headers in `include/`.
The `-stm32` step is an STM32CubeIDE project: see its README, "Working with the step".

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the
  PlatformIO buttons, or from the terminal: `pio run -d 00-practice-blink-esp32`.
- **Serial Monitor:** the UART USB-C port, 115200 baud (set in `platformio.ini`).
