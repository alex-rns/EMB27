# Lesson 2.2 — Passive and active loads: how to connect a heavy load

[← Module 2](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** ESP-IDF in C (PlatformIO, `framework = espidf`)

A transistor switch drives a relay module from a GPIO; the relay's dry contact is read back on another GPIO to
measure the relay's mechanical delay and see its contact bounce.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|

## Working with the steps

Every ESP32 step is an ordinary PlatformIO project on ESP-IDF, written in C: `platformio.ini`, `src/main.c`, the two
`CMakeLists.txt` the PlatformIO wizard makes for ESP-IDF, and `sdkconfig.defaults` (ESP-IDF's settings; the full
`sdkconfig.<env>` is generated at build time and not tracked).

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the PlatformIO buttons,
  or from the terminal: `pio run -d <step-folder>`.
- **Serial Monitor:** the UART USB-C port, 115200 baud (set in `platformio.ini`).
