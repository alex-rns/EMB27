# 03 — Base: mode 1 while the button is held

**Assignment, mode 1 ("fast / synchronous"):** selected by the external button; both LEDs turn on and off
together with `delay(200)`.

First stage of the homework: while the external button is held, both LEDs blink together, 200 ms on and
200 ms off; when it is released they stay dark. Latching the mode (it stays after the release, and mode 1
is active at start) comes in [05](../05-base-combined/).

| Part | GPIO |
|---|---|
| Green LED (through 220 Ω, cathode → GND) | 15 |
| Blue LED (through 220 Ω, cathode → GND) | 16 |
| Button (other leg → GND) | 21 |

![Circuit in Wokwi](diagram.png)

**Notes**
- `INPUT_PULLUP` + a button to GND: released = HIGH, pressed = LOW. A button to 3V3 would change nothing.
- The button is read once per 400 ms cycle, so after a release the LEDs finish the current cycle first.
- A tactile button has four legs connected in pairs: put it across the breadboard gap and use diagonal legs,
  or it reads as always pressed.
- GPIO 21 sits next to 19/20 (native USB) on the header: a wire one hole off drops the USB connection.
