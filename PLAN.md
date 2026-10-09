# Learning plan: C → Embedded

> Companion to `CLAUDE.md`. This is what each item actually covers.
> The unit is a **lesson**, not a week: one step at a time, no schedule.
> Mark items ✅ as they are completed.

---

## 🔴 Block 1 — C for embedded

What interviews ask about, and what you need to spot mistakes in generated code.

### 1. `volatile` ✅

Covered: CPU registers as a "notepad", why the compiler caches values;
the as-if rule; three cases (hardware register, ISR, DMA); `volatile` ≠ atomicity;
read-modify-write and race conditions; `volatile` controls the compiler, not the hardware cache.

**Interview questions:** what `volatile` forbids; where it is needed; why `counter++` is still dangerous.

### 2. Bits in a register ✅

Covered: a register = 32 switches; hex because 1 digit = 4 bits; `1u << n`;
`|=` / `&= ~` / `^=` / `&`; the test yields 2ⁿ, not 1; the `=` instead of `|=` trap;
multi-bit fields (clear → write); mask `(1u << n) - 1`.

**Questions:** write set/clear/test of a bit; why a 2-bit field takes two lines.
**Open thread:** 5 check questions not answered yet.

### 3. Types as sizes — `uint8_t`, `uint32_t`

- why embedded code avoids `int`: its size depends on the platform
- `stdint.h`: `uint8_t`, `int8_t`, `uint16_t`, `uint32_t`, `size_t`
- `sizeof` and what it actually counts
- overflow: unsigned wraps around (well-defined), signed is **undefined behavior**
- integer division, why `1/2 == 0`

**Questions:** how many bytes is an `int` on STM32; what happens with `uint8_t x = 255; x++`.
**Where AI code breaks:** `int` instead of `uint32_t` when working with a register; a counter that silently overflows.

### 4. Pointers and register access

- what an address is; `&` and `*`
- a pointer = a variable that stores an address
- casting a number to an address: `(volatile uint32_t *)0x40020014`
- breaking down `#define GPIOA_ODR (*(volatile uint32_t *)0x40020014)` symbol by symbol
- `volatile uint32_t *p` vs `uint32_t * volatile p`

**Questions:** what this `#define` does; why `volatile` is here; why `uint32_t` and not `uint8_t`.
**This is the line you will see every day.**

### 5. `static` / `const` / `extern`

- three different meanings of `static`: a local variable that survives the call; a function visible only within its file; a global not visible from outside
- `const` and where it actually puts the data (flash vs RAM)
- `extern` — the link to what we saw during linking (`T` / `U` symbols)
- translation unit: why `static` in a `.c` file isolates

**Questions:** name the three meanings of `static`.

### 6. Memory: stack, `.data`, `.bss`, heap

*(merged items 6 and 8 from the list — it's one topic)*

- memory map: what lives in flash, what in RAM
- `.text` and `.rodata` → flash; `.data` → RAM (copied from flash at startup); `.bss` → RAM (zeroed)
- the stack: grows down, fixed size (~8 KB on STM32), no OS to catch an overflow
- heap and `malloc`: why embedded avoids it — fragmentation, non-deterministic timing, nothing catches the error
- returning the address of a local variable — a classic disaster

**Questions:** where `const char msg[] = "hi"` ends up; what a stack overflow is without an OS; why `malloc` is avoided.

### 7. Interrupts (ISR)

- what an interrupt physically is; the vector table
- what you **must not** do in an ISR: long work, `printf`, `malloc`, blocking calls
- variables shared with `main` → `volatile` + critical section
- disabling interrupts as a synchronization primitive
- priorities, nesting

**Questions:** what you must not do in an ISR and why; how to safely pass data from an ISR to `main`.

### 8. `struct` and padding

- how a struct is laid out in memory
- alignment: why `sizeof` is bigger than the sum of the fields
- `__attribute__((packed))` and when it is needed
- **why this is critical for protocols:** a struct overlaid on a UART/SPI buffer will read garbage if it has padding
- byte order (endianness) when exchanging data between devices

**Questions:** why `struct { uint8_t a; uint32_t b; }` takes 8 bytes, not 5.

### 9. Arrays, strings, `char *`

- an array in C doesn't know its length; nobody checks the bounds
- a string = bytes up to the zero byte; `strlen` counts every time
- fixed-size buffers, `snprintf` instead of `sprintf`
- typical buffer overflows

**Questions:** how `char buf[32]` differs from `char *p`.

### 10. Signed/unsigned traps

- comparing signed with unsigned: `for (int i = 0; i < size; i++)` when `size` is a `size_t`
- shifting a negative number, `1 << 31` on a signed `int` = UB
- implicit type conversions

**Questions:** why `-1 > sizeof(x)` is true.

---

## 🟡 Block 2 — peripherals

In parallel with the course. The goal is to **understand what should happen and catch it when it doesn't**.

| Topic | Know | Catch in AI code |
|---|---|---|
| **GPIO** | input/output, pull-up/pull-down, push-pull vs open-drain | forgotten pull-up, pin mode not set, clock not enabled |
| **Interrupts** | priorities, debounce | heavy logic in the ISR, shared variable without `volatile` |
| **Timers / PWM** | prescaler, period, duty cycle | frequency calculated for a different clock than yours |
| **ADC** | resolution, reference voltage, averaging | no calibration, reference voltage ignored |
| **UART** | baud, framing, `\n` vs `\r\n` | baud mismatch = garbage in the terminal |
| **I2C** | addressing, ACK, **mandatory pull-up resistors** | AI can't see your board — it won't add the resistors |
| **SPI** | MOSI/MISO/SCK/CS, modes 0–3 | wrong mode = garbage instead of data |
| **DMA** | what problem it solves, cache coherency | local buffer (vanishes from the stack), missing `volatile` |
| **Datasheet** | find the register map and electrical characteristics | addresses from a different chip revision |

---

## 🟡 Block 3 — tools

| Tool | Minimum |
|---|---|
| **Logic analyzer** | capture I2C/SPI/UART, decode, compare with what you intended. **The main way to verify AI code** |
| **GDB + OpenOCD/SWD** | breakpoint, inspect a variable, inspect a register, read the stack |
| **Multimeter** | voltage in parallel, current in series |
| **Makefile / CMake** | read and modify; build a project without PlatformIO |
| **Linker script** | read it; understand which section goes where |
| **Unity / Ceedling** | unit tests for firmware logic on the host — **your advantage from web** |

---

## Block 4 — portfolio

Three projects on GitHub. At least one bare-metal, without Arduino libraries.

**1. The course final project (ESP32).**
Whatever comes out of the course. Must have: a proper README, a wiring diagram, photos.

**2. STM32 bare-metal.**
Blink + UART, **without CubeMX**, own Makefile, own linker script,
peripheral access via registers following the reference manual.
This is the project interviewers ask about most often.

**3. Something with tests.**
A ring buffer or a protocol parser + unit tests in Unity that run on the host in CI.
Rare in junior portfolios — that's exactly why it stands out.

---

## Order

Block 1 — one item at a time, it's the foundation.
Block 2 runs in parallel with the course (the course gives practice, we give understanding).
Block 3 kicks in when the hardware arrives.
Block 4 is assembled at the end of the course from what has already been done.
