# 00 — Lecture blink (course demo)

Example code from the lecture. Three modes chosen at compile time with `#if` flags:
plain GPIO blink, LEDC PWM fade, or the onboard addressable RGB LED (`neopixelWrite`).
The RGB mode is enabled.

**Notes**
- With every flag at `0u` the preprocessor removes all branches and `loop()` is empty.
- `pinMode()` after `ledcAttachPin()` reconnects the pin to plain GPIO and silently stops PWM.
- The Arduino variant puts the RGB LED on GPIO 48, but on DevKitC-1 v1.1 it is on GPIO 38.
  The Wokwi circuit therefore uses an external NeoPixel on GPIO 48.
