# 04 — Extra: variable speed

The pattern from [03](../03-extra-third-led/) speeds up and slows down like a siren:
the flash length moves between 100 ms and 15 ms by 10 ms per cycle.

Wiring is the same as [03](../03-extra-third-led/).

**Notes**
- A `#define` is a compile-time text substitution and cannot change at runtime,
  so the speed is a variable.
- The variables are global because `loop()` is called again and again;
  a local variable would reset on every call.
- `int`, not `uint32_t`: an unsigned value that goes below zero wraps to ~4.29 billion,
  and `delay()` would then wait about 49 days.
