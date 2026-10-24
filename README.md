# EMB27 — Embedded Development course

[![build](https://github.com/alex-rns/EMB27/actions/workflows/build.yml/badge.svg)](https://github.com/alex-rns/EMB27/actions/workflows/build.yml)

Homework and projects from the [Beetroot Academy "Embedded Development"](https://beetroot.academy/courses/online/kurs-embedded-development)
course, group EMB27 (started September 2026, 5 months).

**Boards:** ESP32-S3 DevKitC-1 clone (N16R8): C++ on the Arduino framework, built with PlatformIO, simulated in Wokwi ·
WeAct Black Pill (STM32F411CE) + ST-Link V2: STM32 HAL, built with STM32CubeIDE (from lesson 1.8)

## Modules

| # | Module | Topics | Status |
|---|---|---|---|
| 1 | [Measurements, circuits, basic electronics](module-1/) | GPIO, datasheet, button, ADC, soldering | in progress |
| 2 | [Meet the microcontroller](module-2/) | C++ for MCUs, superloop, interrupts, timers, debounce, `volatile` | in progress |
| 3 | Peripherals and the physical world | ADC calibration, filtering, DAC, PWM, servo, encoder | planned |
| 4 | Communication protocols | UART, I2C, SPI, BME280, DMA, JTAG/SWD | planned |
| 5 | PCB design and control | KiCad, Gerber, RC filters, PID | planned |
| 6 | Architecture, RTOS, power | state machines, FreeRTOS, mutexes, sleep modes, heap | planned |
| 7 | Final project | integration, CI/CD, documentation, optional STM32 port | planned |

Module 0 (self-study before the course) is not part of this repository.

## Repository layout

```
module-N/                     one folder per course module
├── README.md                 topics of the module and its lessons
└── N.M-<topic>/              one folder per lesson with homework
    ├── README.md             the assignment and a table of its steps
    └── NN-<type>-<slug>/     one step = one ordinary PlatformIO project (ESP32)
        ├── platformio.ini
        ├── src/main.cpp      the firmware
        ├── diagram.json      Wokwi circuit
        ├── diagram.png       screenshot of the circuit
        ├── wokwi.toml        Wokwi config
        └── README.md         wiring and what the step shows
```

Step types, numbered from `00` in the order of the work: `practice` (own practice before the class, optional),
`lecture` (demo from the class), `base` (required homework), `extra` (optional harder steps).
From module 2 on a step's name ends with its board: `-esp32` (the PlatformIO project above) or `-stm32`
(an STM32CubeIDE project: `.ioc`, `Core/`, `Drivers/`). The same number on both boards is the same task.

## Build and run

1. Open a step's folder in VS Code with the PlatformIO and Wokwi extensions, then **Build** or **Upload**.
   From the terminal: `pio run -d module-1/1.3-esp32-blink/05-extra-one-gpio`.
2. To simulate, build the step first, then open its `diagram.json` and press ▶.

3. STM32 steps open in STM32CubeIDE: see [Working with the steps](module-1/1.8-stm32-blink/README.md#working-with-the-steps)
   in lesson 1.8.

Every project is built with `-Wall -Wextra` for its own sources.
