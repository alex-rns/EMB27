# EMB27 — Embedded Development course

Homework and projects from the [Beetroot Academy "Embedded Development"](https://beetroot.academy/courses/online/kurs-embedded-development)
course, group EMB27 (started September 2026, 5 months).
I'm a Ruby on Rails / TypeScript developer moving into embedded systems.

**Board:** ESP32-S3-DevKitC-1 · **Language:** C++ (Arduino framework) · **Build:** PlatformIO · **Simulation:** Wokwi

## Modules

| # | Module | Topics | Status |
|---|---|---|---|
| 1 | [Measurements, circuits, basic electronics](module-1/) | GPIO, datasheet, button, ADC, soldering | in progress |
| 2 | Meet the microcontroller | C++ for MCUs, superloop, interrupts, timers, debounce, `volatile` | planned |
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
└── N.M-<topic>/              one PlatformIO project per lesson
    ├── README.md             the assignment and a table of its tasks
    ├── platformio.ini        one environment per task
    └── tasks/NN-<type>-<slug>/
        ├── main.cpp          the firmware
        ├── diagram.json      Wokwi circuit
        ├── wokwi.toml        Wokwi config for this task
        └── README.md         wiring and what the task shows
```

Task types: `lecture` (demo from the class), `base` (required homework), `extra` (optional harder steps).

## Build and run

1. Open `EMB27.code-workspace` in VS Code with the PlatformIO and Wokwi extensions.
2. Pick a task's environment in the PlatformIO status bar, then **Build** or **Upload**.
   From the terminal: `pio run -d module-1/1.3-esp32-blink -e 05-extra-one-gpio`.
3. To simulate, build the task first, then open `tasks/<task>/diagram.json` and press ▶.

Every project is built with `-Wall -Wextra` for its own sources.
