# Lesson 1.5 — Signals and a button: what an oscilloscope sees. Homework: button bounce on a logic analyzer

[← Module 1](../)

**Board:** ESP32-S3 DevKitC-1 clone (N16R8) · **Framework:** Arduino (PlatformIO) · **Simulator:** Wokwi ·
**Logic analyzer:** 8-channel 24 MHz Saleae Logic clone (Logic 2, PulseView)

**Assignment:** measure the contact bounce of a button and compare the real number of pulses (logic analyzer)
with what the microcontroller catches through a hardware interrupt. A button with a pull-up on GPIO 16, the
analyzer's channel 1 on the same pin. The given program counts `FALLING` interrupts and prints the count.
Press the button 10 times with different speed and force and record every press: how many bounce pulses really
happened, and how long the bounce lasted. Then answer: do the analyzer and the counter agree; when does the
microcontroller miss bounce and when does it count too much; what is the average bounce of this button; what
minimal debounce would be enough.

## Steps

| # | Step | Type | What it shows |
|---|---|---|---|
| 00 | [lecture-debounce](00-lecture-debounce/) | lecture demo | debounce with a 30 ms quiet window; the button, what the firmware saw, the debounced level and a reference square wave on four analyzer channels |
| 01 | [base-interrupt-counter](01-base-interrupt-counter/) | base | the homework: a `FALLING` interrupt counter against the analyzer over 10 presses, bounce per press |

## Working with the steps

Every step is an ordinary PlatformIO project: `platformio.ini`, `src/main.cpp`, and a Wokwi circuit.

- **Build / Upload:** open the step's folder in VS Code (or add it to the workspace) and use the
  PlatformIO buttons, or from the terminal: `pio run -d 00-lecture-debounce`.
- **Simulate:** build the step first, then open its `diagram.json` and press ▶. Wokwi simulates the bounce
  of its pushbutton, and its logic analyzer part records the channels.
- **Serial:** both steps print at 115200 baud; read them on the board's UART USB-C port.
- **Captures:** `00` has a PulseView capture (`capture.sr`), `01` a Logic 2 capture (`capture.sal`).
  The two programs can't share the analyzer: close one and replug the analyzer before using the other.
