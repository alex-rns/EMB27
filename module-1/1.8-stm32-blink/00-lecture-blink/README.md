# 00 — Lecture: blink the Black Pill's LED while its button is held

The first STM32 program. CubeMX sets up the clock and the pins and generates the init code; the loop, written by hand
between the `USER CODE` markers, reads the board's KEY button and toggles the onboard blue LED while it is held:

```c
while (1)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        HAL_Delay(100);
    }
}
```

The lecture first blinked the LED with no condition (`HAL_Delay(50)`, the step's first commit), then added the button.
The teacher named PA0 `BUTTON1` in CubeMX, which generates `BUTTON1_GPIO_Port` and `BUTTON1_Pin`; this step keeps
the default names, `GPIOA` and `GPIO_PIN_0`.

| Pin | CubeMX setting | Used for |
|---|---|---|
| PC13 | GPIO output, push-pull, no pull, low speed | the onboard blue LED |
| PA0 | GPIO input, pull-up | the board's KEY button: released = HIGH, pressed = LOW |
| PA13 / PA14 | SYS: Serial Wire | SWDIO / SWCLK for the ST-Link |
| PH0 / PH1 | RCC: HSE crystal | 25 MHz crystal |
| PC14 / PC15 | RCC: LSE crystal | 32.768 kHz crystal |

**Clock:** HSE 25 MHz → PLL (M = 12, N = 96, P = 2) → SYSCLK 100 MHz, AHB 100 MHz, APB1 50 MHz.

**Wiring:** only the ST-Link, to the 4-pin SWD header at the end of the board, by the printed labels:
ST-Link 3.3V → 3V3, GND → GND, SWDIO → DIO, SWCLK → SCK. The board then runs from the ST-Link's 3.3 V,
so its USB-C stays unplugged. The LED and the button are on the board:

```
3V3 ── 1.5 kΩ ── LED ──┤ PC13
GND ── KEY ── 330 Ω ───┤ PA0    (the pull-up to 3.3 V is inside the chip)
```

## What breaks on real hardware

- **PA0 needs the pull-up.** Nothing on the board ties PA0 to 3.3 V: the KEY button can only pull it to GND. With
  CubeMX's default `No pull-up and no pull-down`, a released button leaves PA0 floating, and the input reads whatever
  noise and leakage push it to, so the LED may toggle on its own. Before the pull-up was set, PA0 read 1 five times
  in a row with the button released: luck, not a guarantee. The internal pull-up (30–50 kΩ, datasheet) holds it at 1;
  a press pulls it to 0 through 330 Ω.
- **The LED is lit by LOW.** Its anode sits on 3.3 V through 1.5 kΩ and PC13 pulls the cathode down, so
  `ODR13 = 1` means off. CubeMX writes PC13 low before it switches the pin to output, so the LED comes on at reset.
- **`HAL_Delay(100)` waits 101 ms.** HAL adds one tick (1 ms) so the wait is never shorter than asked: while the
  button is held, the LED toggles every 101 ms, a full blink takes 202 ms (about 5 Hz). The tick comes from the
  SysTick interrupt; with interrupts off, or inside an interrupt of higher priority than SysTick, `HAL_Delay` never
  returns.
- **`HAL_Delay` blocks.** The CPU spins in a loop for those 101 ms and doesn't read the button: a tap toggles the LED
  once, and the LED stays in whatever state it had when the button was released.
- **PA13/PA14 must stay SWD.** Configuring them as GPIO in CubeMX cuts the debugger off after the next flash;
  then only a connect under reset (or the ROM bootloader) gets the ST-Link back in.
- **USB needs a different clock tree.** USB OTG needs exactly 48 MHz; at SYSCLK 100 MHz this PLL gives 50 MHz on
  that output. A step that enables USB has to change the clock settings in CubeMX first.
- **A regenerate rewrites everything outside `USER CODE`.** Code placed elsewhere in `main.c` disappears without
  a warning. The pull-up is a CubeMX setting, so it lives in the `.ioc` and in the generated `MX_GPIO_Init()`.

## Measured on the board

- Build (CubeIDE 2.2.0, `-O0 -g3`, Debug, clean rebuild): 0 errors, 0 warnings. Flash: 5996 bytes
  (`arm-none-eabi-size -A`: the vector table `.isr_vector` 408, code `.text` 5556, constants `.rodata` 16, the rest
  init/fini tables). RAM: variables take 44 bytes (`.data` 12, `.bss` 32); `._user_heap_stack` reserves 1540 more for
  the heap (0x200) and the stack (0x400) set in CubeMX's Project Manager (`_Min_Heap_Size`, `_Min_Stack_Size` in the
  linker script).
- Flashing the `.elf` through the ST-Link: erase sector 0 (16 KB, enough for the whole program), write, verify, reset.
- Read over SWD without halting the core:
  - `GPIOA_PUPDR` (`0x4002000C`) = `0x64000001`: `PUPDR0` = `01`, pull-up (it read `00` before the pull-up was set).
    `0x64000000` is the register's reset value: the pull-ups and the pull-down of the debug pins PA13–PA15.
  - `GPIOA_IDR` bit 0 = 1 and `GPIOC_ODR` bit 13 = 0 on every read with the button released: PA0 is high and the loop
    leaves the LED alone (lit).
  - `GPIOC_MODER` = `0x04000000`: `MODER13` = `01`, output.
- `-Wextra` (the repo's usual flag) is not in CubeIDE's defaults; a test build with it gave no warnings in `Core/`
  and 3 in the HAL (`stm32f4xx_hal_flash_ex.c`, unused parameter `Banks`).
