# Project Notes

## Sessions 1–2 — Boot: linker script, startup code, vector table

### What was accomplished

- Wrote a GNU ld linker script from scratch: `MEMORY` block defining FLASH
  (256K at `0x0`) and SRAM (64K at `0x20000000`), plus a `SECTIONS` block
  placing `.vector_table`, `.text`, `.data`, and `.bss` in the correct regions.
- Defined `_stack_top` as a linker symbol pointing to the top of SRAM.
- Wrote `startup.c` containing the vector table (as a `const` array in the
  `.vector_table` section) and a `Reset` handler that copies `.data` from
  FLASH to SRAM, zeros `.bss`, then calls `main()`.
- Wrote a hand-built `Makefile` with separate compile and link steps, plus
  `run.sh` and `debug.sh` helper scripts.
- Confirmed the firmware boots and runs on QEMU (`lm3s6965evb`) with no
  crash or lockup.

### What was hard

The linker script took the most effort — understanding the difference between
VMA (runtime address) and LMA (load address), how `AT()` places `.data`
initial values in FLASH while the section runs in SRAM, and how `> FLASH` /
`> SRAM` region specifiers work.

Connecting the linker script to C code was also non-obvious. The SECTIONS
block defines symbols (`_stack_top`, `_data`, `_edata`, etc.) that startup
code needs to reference.

### Mistakes to remember

**Linker symbols are addresses, not values.** When you declare
`extern char _symbol;` in C, you must use `&_symbol` to get the address the
linker assigned. Writing `_symbol` (without `&`) reads the value stored *at*
that address — which is meaningless for a linker symbol that has no storage.

This tripped up the vector table (`_stack_top`) and the `.data` / `.bss`
pointers in `Reset` multiple times.

### Surprises

The overall structure of the linker script and the reset routine turned out
to be more straightforward than expected once the concepts clicked. The hard
part was not the complexity of the code but the unfamiliar mental model:
memory regions, section placement, and the VMA/LMA distinction are not
concepts that appear in hosted C programming.

---

## Session 3 — GPIO: register access, clock gating, volatile

### What was accomplished

- Identified the four registers needed to drive a GPIO pin on the LM3S6965:
  RCGC2 (clock enable), GPIODIR (direction), GPIODEN (digital enable), and
  GPIODATA (data, with pin mask baked into the address).
- Looked up all base addresses and offsets from the LM3S6965 datasheet
  (Chapter 5 for RCGC2, Chapter 8 for GPIO Port F).
- Defined the registers as `volatile uint32_t *` macros in `lm3s6965.h`.
- Initialized PF0 as a digital output in `main()` and set it high — firmware
  boots and runs on QEMU without faulting.
- Set up `compile_flags.txt` so clangd provides correct autocomplete in vim
  for the cross-compilation target.

### What was hard

Navigating the datasheet was the main challenge — knowing which chapter to
look in, which register table to read, and how to interpret multi-column bit
descriptions. The programming side was less difficult than expected once the
addresses were known.

### Mistakes to remember

**`|=` sets bits, `&=` clears them.** To enable a peripheral clock bit
without disturbing others, the correct operation is `*RCGC2 |= (1 << 5)`,
not `&=`. Using `&=` with a single-bit mask would clear every other bit in
the register.

**GPIODATA addressing.** The data register address encodes a pin mask in
bits [9:2]. To write only to pin 0, the offset is `0x01 << 2 = 0x004`, not
`0x000`. Using the base address alone writes through a zero mask and affects
nothing.

### Surprises

`volatile` — the keyword that prevents the compiler from optimizing away
writes to memory-mapped registers — was a new concept. Without it, the
compiler could legally discard register writes entirely, since from its
perspective nothing in the program reads the result.

### To explore further

Reading complex hardware documentation (datasheets, TRMs) more fluently —
specifically: how to locate the right chapter quickly, how to read register
bit tables, and how to cross-reference the memory map with register
descriptions. Worth practising deliberately on the next peripheral.

---

## Session 4 — UART: serial output, baud rate, alternate function pins

### What was accomplished

- Identified all registers needed for UART0: RCGC1 (clock), GPIOAFSEL
  (alternate function), GPIOA_DEN (digital enable on Port A), UARTCTL
  (enable), UARTIBRD/UARTFBRD (baud rate divisors), UARTLCRH (line
  control), UARTFR (flags), UARTDR (data).
- Calculated baud rate divisors for 115200 baud at 12MHz system clock:
  IBRD = 6, FBRD = 33.
- Wrote `uart_init()` and `uart_putc()` in `uart.c`/`uart.h`.
- Added `-nographic` to `run.sh` to route UART0 output to the terminal.
- Confirmed `Hi` prints to the terminal on QEMU — first visible output
  from bare-metal code.

### What was hard

Nothing was as hard as the linker script. Datasheet navigation is
getting faster with practice — finding addresses and relevant bit fields
is becoming more routine.

### To improve

**Register definition structure.** The current approach defines each
register as a separate macro with a hardcoded full address. A cleaner
pattern is to define a base address per peripheral and express each
register as `(BASE + offset)`. This makes the grouping explicit and
makes adding UART1/UART2 a one-line change. Worth refactoring before
the project grows further.

**Peripheral initialization from the datasheet alone.** The
"Initialization and Configuration" subsection in each peripheral chapter
lists the exact sequence of steps needed. Goal for next peripheral:
find that section first and derive the init sequence independently
before asking for guidance.

### New concepts

**Baud rate calculation.** The UART divides the system clock by
`16 × BRD` to produce the baud rate. The divisor has integer
(UARTIBRD) and fractional (UARTFBRD) parts. Fractional part =
`round(frac × 64)`.

**Alternate function pins.** GPIO pins default to plain GPIO mode.
To connect them to a peripheral (UART, SPI, etc.), GPIOAFSEL must
be set for those pins — this hands control of the pin to the peripheral.
GPIODEN must also be set to enable the digital function.
