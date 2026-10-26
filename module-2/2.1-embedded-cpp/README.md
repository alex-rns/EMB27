# Lesson 2.1 — C++ for MCUs: resource limits and specifics. Homework: Embedded C++ blink

[← Module 2](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** Arduino core 2.0.17 (PlatformIO)

Embedded C++ on the ESP32: `enum class`, `constexpr`, a `Led` class, a non-blocking superloop on `millis()`,
the loop's own timing, and a button on an interrupt.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [practice-blink-esp32](00-practice-blink-esp32/) | practice | the homework tasks done once before the lecture |

## Working with the steps

Every step is an ordinary PlatformIO project: `platformio.ini`, `src/main.cpp`, headers in `include/`.

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the
  PlatformIO buttons, or from the terminal: `pio run -d 00-practice-blink-esp32`.
- **Serial Monitor:** the UART USB-C port, 115200 baud (set in `platformio.ini`).
