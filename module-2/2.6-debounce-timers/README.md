# Lesson 2.6 — Debounce: reliable input with timers

[← Module 2](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8), ESP-IDF in C (PlatformIO)

Contact bounce and how to filter it: a level counts only after it has stayed stable for a while, and a state machine
turns the stable presses into gestures (short, double, long, stuck).

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-button-fsm-esp32](00-lecture-button-fsm-esp32/) | lecture | the teacher's demo: a 10 ms polling task and a debounced button state machine with SHORT / DOUBLE / LONG / STUCK, plus its STM32 snippet |
| 01 | homework-button-fsm: [esp32](01-homework-button-fsm-esp32/) | homework (extra) | the lecture state machine rewritten as a switch-case over explicit states, with a callback per event (`button_fsm_on()`); one commit per change, the first one is the lecture code ([assignment](01-homework-button-fsm-esp32/README.md#assignment)) |

## Working with the steps

Every `-esp32` step is an ordinary PlatformIO project on ESP-IDF: `platformio.ini`, `CMakeLists.txt`, the code in
`src/`.

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the PlatformIO buttons,
  or from the terminal: `pio run -d 00-lecture-button-fsm-esp32`.
- **Serial Monitor:** the UART USB-C port, 115200 baud (set in `platformio.ini`).
