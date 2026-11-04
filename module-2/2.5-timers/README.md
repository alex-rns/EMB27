# Lesson 2.5 — Timers: how an MCU measures time precisely

[← Module 2](../)

**Board:** ESP32-S3 DevKitC-1 clone, ESP-IDF, C

A hardware timer counts on its own and calls our code at the moment we set: the main loop no longer keeps the time.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | homework-exhaust-fan: [esp32](00-homework-exhaust-fan-esp32/) | homework | a GPTimer alarm switches a relay (the fan) on for `FAN_ON_S` every `FAN_PERIOD_S`, from its interrupt callback, with no loop or task; a state log and the task watchdog ([assignment](00-homework-exhaust-fan-esp32/README.md#assignment)) |

## Working with the steps

Every `-esp32` step is an ordinary PlatformIO project on ESP-IDF: build it with `pio run -d <step folder>`, and see its
README, "Working with the step".
