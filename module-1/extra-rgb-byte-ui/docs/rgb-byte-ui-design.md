# RGB byte UI: eight LEDs, six buttons, an RGB LED — design

Extra mini project, Module 1, EMB27 course (not part of the course program). Status: approved by the author
2026-10-20; the same day the steps were re-split (selection and slots became two steps, 00–07). Plan:
[`rgb-byte-ui-plan.md`](rgb-byte-ui-plan.md).

## Intent

Eight LEDs in a row show one byte. With six buttons the user builds three bytes, one for each colour of an RGB LED,
bit by bit, and then lights the RGB LED with them. It is a hardware colour picker: the same three numbers as CSS
`rgb(160, 64, 0)`, entered in binary.

Why it is built this way:
- every bit operation the project needs (shift, set, clear, toggle, test) is visible on the LEDs at once;
- it stays at the Module 1 level: buttons are polled in `loop()` with a software debounce, as in lesson 1.5;
  no interrupts or hardware timers are required;
- it is built in small steps: each step is a working program that adds one new thing, checked on the board before
  the next one.

This document describes behaviour and hardware only. It deliberately says nothing about how the code is structured.

## The assignment (translated in full)

Title: "A UI that shows the bytes for RGB".

> Eight LEDs in a row: a UI that shows the bytes for RGB that the user enters.
>
> Button 1 shifts THE BIT cyclically to the left.
> Button 2 stores the active bit at its position.
> Buttons 3–5 store the current byte as the code for R, G, B respectively.
>
> When button 3–5 is pressed, the UI indicates the current setting. On the second and later presses it stores.
>
> Button 6 lights the last stored configuration on the RGB LED.

## Behaviour (the finished project)

Agreed with the author on a clickable mockup of this behaviour.

**The eight LEDs.** Bit 7 is the leftmost LED, bit 0 the rightmost, so the byte reads as it is written on paper
(160 = `1010 0000` = `●○●○○○○○`). They always show the value of the selected slot.

**The cursor (the "active bit").** One bit position is the cursor. At power-on it is on bit 0.
- Steps 01–05: the LED under the cursor is lit steadily, whatever the bit's value.
- From step 06 on: the LED under the cursor blinks at a fixed rate, the same for a 0 and a 1. The other seven LEDs
  show their bits. Accepted consequence: while the cursor sits on a bit, that bit's value is not visible; it shows
  once the cursor moves on.

**Slots.** Three slots, R, G and B, one byte each, 0–255. At power-on all three are 0 and R is selected.

| Button | Does |
|---|---|
| 1 | Moves the cursor one place left (bit n → bit n+1); from bit 7 it wraps to bit 0. Changes no value. |
| 2 | Toggles the bit under the cursor in the selected slot (0 → 1, 1 → 0). The slot changes at once: there is no separate "store" step. |
| 3, 4, 5 | Select slot R, G, B. The eight LEDs switch to that slot's value; the cursor stays where it was. Pressing the button of the slot that is already selected does nothing. |
| 6 | Lights the RGB LED with the three slot values as they are at that moment (each 0–255, by PWM). Later changes to the slots do not reach the RGB LED until button 6 is pressed again. With all slots at 0 the RGB LED stays dark: that is correct. |

**Slot indicators.** One small LED next to each of buttons 3, 4, 5: red next to R, green next to G, blue next to B.
Only the indicator of the selected slot is lit.

**At power-on and after a reset:** slots 0, 0, 0; R selected (red indicator on); cursor on bit 0; RGB LED off.

**Buttons in general.** A press acts once, when the button goes down. Holding a button does nothing more (no
auto-repeat). Contact bounce must not turn one press into two, on the press or on the release (lesson 1.5 measured
the kit's buttons bouncing on releases, 0.07–1.2 ms).

**Memory.** The slots live in RAM. A reset or a power cycle clears them; step 07 keeps them in flash.

**Step 07 is outlined only.** What is written to flash and when, what comes back at boot (the slots only, or also
the selected slot and the cursor), whether the RGB LED lights at boot, and whether the values survive a re-flash are
designed after step 06 works. A candidate to start the discussion from: write the three slots when button 6 is
pressed (few flash writes, and it matches the assignment's "the last stored configuration"), restore them at boot.

### Where this departs from the assignment, and why

1. **No separate "store" press (autosave).** The assignment's "the first press shows the current setting, the
   second and later presses store" is replaced: a press of 3–5 selects the slot and shows its current value (the
   first half survives), and button 2 changes the selected slot directly, so nothing is left to store. On a mockup
   that followed the assignment literally, a byte typed before choosing a slot was replaced by the slot's stored
   value on the first press of 3–5, and a press of button 6 with nothing stored looked like a broken button.
   Autosave removes both traps.
2. **Three slot indicators added.** Without them nothing on the board shows which slot is selected.
3. **The cursor is drawn on top of the byte.** The assignment does not say how the active bit is shown. Steady in
   the first steps, blinking from step 06.

## Steps

The whole project is one ordinary PlatformIO project, `module-1/extra-rgb-byte-ui/`, and each step is one commit:
the git history shows the progress (decided by the author on 2026-10-20, instead of one folder per step). No Wokwi
circuit per step. The hardware grows with the code: only what the step needs is on the breadboard.

| Step | What you see on the board | Hardware added |
|---|---|---|
| `00-base-byte-leds` | The eight LEDs show a byte fixed in the code (for example 160). Changing the number and re-flashing changes the pattern. | 8 LEDs, 8 × 220 Ω |
| `01-base-cursor` | Button 1 moves a single lit LED left, wrapping from bit 7 to bit 0. Nothing else is lit. | button 1 |
| `02-base-toggle-bit` | Button 2 toggles the bit under the cursor; the LEDs show the byte with the cursor lit on top. | button 2 |
| `03-base-select` | Buttons 3/4/5 select R/G/B and light their indicator. There is still one byte: the eight LEDs don't change on a selection. | buttons 3–5, 3 indicator LEDs, 3 × 220 Ω |
| `04-base-slots` | Each slot keeps its own byte: button 2 changes the selected slot, a selection shows that slot's byte. | — (logic only) |
| `05-base-rgb` | Button 6 lights the RGB LED with the three slots. This completes the assignment's behaviour. | RGB LED, 3 × 220 Ω, button 6 |
| `06-extra-blink` | The cursor blinks; no press is lost while it blinks. | — |
| `07-extra-remember` | Optional. The slots survive a reset and a power cycle. | — |

The step names label the commits. Inside a step, smaller
checkpoints come first and are not saved on their own: in step 01, one Serial line per press before the cursor moves;
in step 05, the diode test and a fixed colour from the code before button 6.

## Hardware

**Parts (all in the course kit or the AliExpress kit; counted on 2026-10-20):**

| Part | Count | Note |
|---|---|---|
| 3 mm LED, one colour for the byte | 8 | one colour, so all eight look equally bright; plenty on hand |
| 3 mm LED red, green, blue (slot indicators) | 1 each | plenty on hand |
| RGB LED, discrete, 4 legs | 1 | AliExpress kit; common anode or common cathode is not stated: see below |
| Tactile button | 6 | exactly 6 on hand, no spare |
| 220 Ω resistor | 14 | 8 + 3 + 3; plenty on hand |
| Breadboard, jumper wires | | 20 signal wires to the board in the end |

**Pins.** 20 GPIOs in the end: 8 byte LEDs, 6 buttons, 3 indicators, 3 RGB channels. Choose them from
1, 2, 4–18, 21, 38–42 (23 pins). Avoid 0, 3, 45, 46 (strapping), 19/20 (native USB), 26–37 (flash/PSRAM), 43/44
(UART0). Leave 47 and 48 unused: which of them drives an onboard LED on this board is still unverified.
Wokwi's DevKitC-1 model has its own RGB LED on GPIO 38, so in the simulation a part on 38 also lights that LED.

**Buttons** go between the GPIO and GND with the internal pull-up enabled: no external resistors, pressed reads LOW.

**LED current.** A red LED with 220 Ω at 3.3 V takes about 6 mA (lesson 1.3); green and blue take less (higher Vf).
All 14 LEDs and channels lit at once draw under 90 mA. Limits: about 20 mA per pin at the default drive strength
(ESP32-S3 TRM, IO MUX table, DRV = 2), 1500 mA cumulative IO output current (ESP32-S3 datasheet, Table 5-1,
absolute maximum).

**The RGB LED.**
- Find out whether it is common anode or common cathode with the multimeter's diode test before wiring it. With a
  common anode, the shared leg goes to 3V3 (not GND) and a colour lights when its pin is LOW, so the PWM duty is
  inverted: 255 means off.
- PWM with Arduino core 2.0.17: `ledcSetup`, `ledcAttachPin`, `ledcWrite` (not the 3.x `ledcAttach`). With 8-bit
  resolution a slot value is the duty as it is.
- On real hardware: perceived brightness is not linear (128 looks much brighter than half of 255), and the three
  colours differ in brightness at the same duty, so a mix will not match the same `rgb()` on a screen. Known effects,
  not to be corrected in this project.
- The project uses the discrete LED. The board's own WS2812 (one pin, a different protocol, a library, its pin still
  unverified) is out of scope; it could become a separate extra later.

## Goals, non-goals, anti-goals

**Goals:** the behaviour above, reached through the steps above, each working on the real board.

**Non-goals:** interrupts or hardware timers for the buttons; shift registers or LED drivers; the onboard WS2812;
any UI over Serial or Wi-Fi; brightness correction (gamma); long press, auto-repeat or button combinations.

**Anti-goals** (the project fails even if it otherwise works):
- a press that is lost or counted twice;
- buttons that stop responding while the cursor blinks (step 06);
- a part on a pin from the avoid list;
- an LED without a series resistor.

## Verification

- Every step: the author runs the step's checks on the board (the presses in the table above and the edge cases:
  the wrap from bit 7 to bit 0, toggling a bit back, selecting the already selected slot, button 6 with all slots
  at 0).
- Step 05: the RGB LED shows red only for `255 0 0`, green only for `0 255 0`, blue only for `0 0 255`; if a colour
  is inverted or swapped, the anode/cathode or the wiring is wrong.
- Step 06: fast presses of button 1 while the cursor blinks move the cursor by exactly the number of presses.
- Step 07: checks are written together with its behaviour. In any version they run on the board: the test is a
  real reset and a real power cycle.

## Left to the builder

Pin numbers (from the pool above), the debounce window, the blink rate, the colour of the eight byte LEDs, the exact
step folder names, and whether a step prints debug output on Serial.
