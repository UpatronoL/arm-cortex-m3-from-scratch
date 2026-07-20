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

---

## Session 5 — SysTick: interrupts, vector table, tick counter

### What was accomplished

- Expanded the vector table from 2 to 16 entries, filling slots 2–14
  with `Default_Handler` and placing `SysTick_Handler` at index 15.
- Created `systick.c` and `systick.h` with `systick_init()`,
  `SysTick_Handler`, and a `volatile uint32_t systick_count` counter.
- Configured SysTick: STRELOAD = 11999 (1ms at 12MHz), STCURRENT = 0,
  STCTRL with ENABLE + INTEN + CLK_SRC.
- Main loop detects 1000 ticks and prints a character via UART —
  confirmed one character per second in the terminal.

### What was hard

Nothing was particularly hard this session. The initialization pattern
is becoming familiar — finding registers, calculating values, writing
the init sequence — and the datasheet navigation was noticeably faster
than previous sessions.

### Mistakes to remember

**Vector table index off by one.** SysTick is exception 15, which means
index 15 in the vector table (0-indexed). With 13 `Default_Handler`
entries filling slots 2–14, `SysTick_Handler` must be the 16th entry.
Having one too few `Default_Handler` entries placed `SysTick_Handler`
in the PendSV slot (index 14) — SysTick fired but jumped to garbage,
silently doing nothing.

**Always count vector table entries against the datasheet layout.**
The Cortex-M3 TRM table is the ground truth — cross-check index numbers
before assuming the handler is in the right slot.

### What clicked

Datasheet navigation is genuinely improving. Registers, base addresses,
and bit fields were found independently and correctly this session
without significant guidance. The "Initialization and Configuration"
subsection pattern is now a reliable starting point for any new
peripheral.

### New concepts

**SysTick reload value.** The timer fires every N+1 cycles where N is
the STRELOAD value. For exactly 12,000 cycles (1ms at 12MHz): STRELOAD
= 11999.

**`volatile` for shared state between main and ISR.** Without
`volatile`, the compiler caches `systick_count` in a register and the
main loop never sees updates from the interrupt handler.

---

## Session 6 — UART RX: ring buffer, interrupt-driven input, sentinel values

### What was accomplished

- Wrote `ring_buffer.h`/`ring_buffer.c`: a fixed-size (64-byte) circular
  buffer with `volatile` `head`/`tail` indices, `rb_push`, `rb_pop`,
  `rb_is_empty`, `rb_is_full`. Wraparound handled with `& (MAXSIZE - 1)`
  since 64 is a power of two.
- Enabled the UART0 RX interrupt (`UARTIM`, `NVIC_EN0`) and wrote
  `UART0_Handler` to push each received byte into the ring buffer.
- Wrote `uart_getc()`: checks `rb_is_empty()` first, returns a sentinel
  if there's nothing to read yet, otherwise pops and returns the byte.
- Wired `main()`'s loop to poll `uart_getc()` and echo back any byte
  received, confirmed working over the QEMU terminal.

### Mistakes to remember

**A sentinel value must survive the full round trip, not just the return
statement.** `uart_getc()` returns `int` specifically so that `-1` (no
data) can be distinguished from a real byte, since a real byte only ever
occupies `0..255`. But the caller in `main()` originally stored that
return value into a `char`:

```c
char c = uart_getc();
if (c) { ... }
```

Two independent bugs stacked here:

1. **Truncation collides the sentinel with real data.** Assigning the
   `int` return value to a `char` truncates it. Since plain `char` is
   *unsigned* on this toolchain (confirmed via
   `arm-none-eabi-gcc -dM -E -x c /dev/null | grep __CHAR_UNSIGNED__`),
   `-1` truncates to `255` — which is nonzero, so the `if(c)` branch
   fired on *every* iteration whether or not a byte had actually
   arrived, spamming garbage output continuously.
2. **Truthiness isn't the same as "not the sentinel."** Even after
   fixing the variable's type to `int`, `if(c)` still tests "is this
   nonzero" — and `-1` is nonzero. The condition has to explicitly
   compare against the sentinel: `if (c != UART_NO_DATA)`.

Both the storage type *and* the comparison had to be fixed together;
fixing only one still leaves the other bug live.

**Don't reuse compiler-reserved identifiers.** Reached for
`__CHAR_UNSIGNED__` as a macro name for the new sentinel, not realizing
it was already the actual predefined compiler macro used to check char
signedness (see mistake #1 above) — reusing it would have silently
redefined something the compiler relies on. Any identifier starting
with two leading underscores (or `_` + a capital letter) is reserved
for the implementation per C11 §7.1.3 — never define your own macros
with that shape. Settled on `UART_NO_DATA` instead: descriptive,
peripheral-scoped, not reserved.

**`EOF` was the wrong name to reach for.** `EOF` means "this stream is
exhausted, permanently" (end of file/socket). An empty ring buffer means
"nothing available *right now*" — a transient, retryable state, not a
terminal one. Borrowing `EOF`'s name would have been technically legal
(same value, `-1`) but semantically misleading at every call site.

### New concepts

**Ring buffer wraparound via bitmask.** With a power-of-two buffer size,
`(index + 1) & (SIZE - 1)` wraps the index back to 0 without a branch or
modulo — cheaper and avoids signed-overflow edge cases modulo would
have with signed indices.

**Single-producer/single-consumer reasoning.** `UART0_Handler` (ISR)
only ever touches `head`; `uart_getc` (foreground) only ever touches
`tail`. That separation is what makes the check-then-pop sequence in
`uart_getc` safe without needing to disable interrupts around it —
worth re-verifying this holds if a second consumer or producer is ever
added.

---

## Session 7 — mini_printf: variadic functions, sign vs. bit pattern

### What was accomplished

- Wrote `mini_printf.c`/`mini_printf.h`: a minimal `printf` supporting
  `%d`, `%u`, `%x`, `%c`, `%s`, built on `stdarg.h` and `uart_putc`.
- Shared structure across the numeric specifiers: extract digits
  least-significant-first into a small stack buffer via repeated
  `% base` / `/= base`, then print the buffer in reverse.
- `%d` handles negative values with a leading `-`; `%u`/`%x` handle
  the full unsigned range correctly, including bit patterns that look
  negative when read as `int`.

### Mistakes to remember

**An always-true condition made a whole branch dead code.** The first
draft of `%x`'s digit-to-character logic was
`if (digit % 10 < 10) ... else ...`. For any `digit` in `0..15`,
`digit % 10` is always in `0..9`, so the condition is always true —
the `else` branch (meant to produce `'A'`–`'F'`) could never run.
Every hex digit 10–15 printed the wrong character (e.g. `':'` instead
of `'A'`) with no compiler warning, since the code was syntactically
fine. Fixed to `if (digit < 10)`.

**Fixing the branch exposed a second bug underneath it.** Once the
`else` branch became reachable, `buffer[index++] = digit + 'A'` was
still wrong — for `digit = 10` that's `10 + 'A'` = `'K'`, not `'A'`.
Needed `(digit - 10) + 'A'` so `digit = 10` maps to offset `0` from
`'A'`. A reminder that fixing a condition doesn't guarantee the code
behind it is correct — worth tracing through concrete values (`digit
= 10`, `digit = 15`) after any branch fix, not just confirming the
branch is now reachable.

**XOR is not negation.** First attempt at getting a negative `int`'s
magnitude for `%d` was `i ^= 0xFF`. `0xFF` only covers the low 8 bits
of a 32-bit `int` — XOR-ing with it leaves the upper 24 bits
untouched, so it doesn't produce anything close to `-i` for most
values. Correct two's complement negation is "flip every bit, then
add one": `i = ~i + 1` (equivalent to the built-in unary `-`).

**`%u`/`%x` aren't signed — don't borrow `%d`'s sign logic for them.**
First pass at `%u`/`%x` copied `%d`'s "if negative, negate and print
`-`" logic wholesale. But `%u`/`%x` display the raw bit pattern of the
argument reinterpreted as unsigned, not a signed magnitude with a
sign — `printf("%x", -1)` should print `ffffffff`, not `-1`. The
actual bug underneath: doing `%`/`/` on a *signed* `int` that holds a
"negative-looking" bit pattern uses signed division, which follows the
dividend's sign and produces garbage digits. Fix was to read the
argument into an `unsigned int` (`va_arg(args, unsigned int)`); the
`int` → `unsigned int` conversion reinterprets the same bits rather
than doing sign-extended math, so `%`/`/` on it behave correctly with
no sign handling needed at all.

**Copy-paste while adapting `%d`'s pattern to `%u` left stale
references to the wrong variable.** After introducing `unsigned int b`
for `%u`, two lines still referenced the old signed variable `i`:
`digit = i % 10` (should read `b`) and `uart_putc(buffer[b])` in the
print loop (should index with the loop variable, not `b` — which by
that point had already been divided down to `0`, so it printed
`buffer[0]` repeated `index` times instead of walking the buffer
backwards). Caught by diffing the `%u` block line-by-line against the
already-correct `%x` block, which has the same shape.

**Forgot to count a character that wasn't inside the usual print
loop.** `mini_printf`'s return value is a running `count` of bytes
written, incremented at every `uart_putc`. The `-` sign for negative
`%d` values is written *before* the digit loop starts, in its own
`uart_putc('-')` call — easy to miss bumping `count` there since it's
not inside either of the two loops where `count++` is the obvious,
visible pattern.

### New concepts

**`int` → `unsigned int` conversion reinterprets bits, it doesn't do
sign-aware math.** In C this conversion is well-defined as "reduce
modulo `2^N`," which for a two's-complement machine is exactly
"reinterpret the same 32 bits as unsigned." This is why casting/
assigning into an `unsigned int` is the right tool for `%x`/`%u`
formatting, instead of manually detecting sign and negating.

### Open / deferred (not yet fixed — filed for later)

- **Trailing `%` at the end of a format string.** If `c++` lands on
  the format string's `'\0'` right after consuming a `%`, the `switch`
  matches nothing, but the enclosing `for` loop's own `c++` still
  fires before the loop condition is rechecked — stepping one byte
  past the null terminator before `*c != '\0'` is evaluated again.
  Only matters if `mini_printf` is ever fed untrusted/malformed format
  strings; deferred since current call sites are all in this file's
  own control.
- **Unknown format specifiers don't consume a `va_arg`.** There's no
  `default` case in the specifier `switch`, so an unrecognized
  specifier prints nothing *and* leaves the corresponding argument
  unconsumed — any specifier after it in the same call reads the
  wrong argument. Same underlying failure mode as the `%u` bug above
  (an argument silently not advancing through `va_arg`), just
  triggered by a different cause.
- **`INT_MIN` negation overflow.** `~i + 1` on `INT_MIN`
  (`-2147483648`) overflows back to `INT_MIN` itself, since two's
  complement has no positive counterpart for the most negative value.
  `%d` with that exact argument would misbehave. Known limitation of
  essentially every hand-rolled `itoa`; not addressed yet.
- No `%%` escape for a literal percent sign.

---

## Session 8 — UART command shell input: line buffering + mini_strtok

### What was accomplished

- `main.c`: a line-buffering loop that pulls bytes one at a time from
  `uart_getc()` into a fixed `char` array, bounds-checked against
  overflow, terminated on `'\r'`, and null-terminated correctly.
- `mini_strtok.c`/`.h`: a hand-rolled, scaled-down `strtok` — pass a
  real buffer on the first call, then `NULL` on subsequent calls to
  keep pulling words out of the same line, using a `static` file-scope
  pointer (`private_buffer`) to remember where the previous call left
  off, the same way the real library function does internally.

### Mistakes to remember

**The same OR/AND (De Morgan's) mistake surfaced three separate
times across this session, in three different lines of code.** Each
time, a stop condition built from two "not equal to a constant" checks
got joined with `||` instead of `&&`:
```c
while ((c = uart_getc()) != '\r' || c != UART_NO_DATA)   // main.c, attempt 1
while ((c = uart_getc()) == '\r' || c != UART_NO_DATA)   // main.c, attempt 2 (polarity flipped too)
while (*(buffer++) != ' ' || *buffer != '\0')             // mini_strtok.c
```
The general rule: "stop if A or B" always translates to "keep going
only while (not A) **and** (not B)." Whenever two different-constant
inequality checks (`x != K1`, `x != K2`) get OR'd together, the result
is a near-tautology — a single value can't simultaneously equal *both*
constants, so at least one side of the `||` is nearly always true, and
the loop never stops when it should. Reach for `&&` whenever combining
inequality checks into a stop condition, not `||`.

**Polling loop stored every empty poll, not just real keystrokes.**
`uart_getc()` returns immediately with `UART_NO_DATA` (`-1`) when
nothing has arrived — it doesn't block. Early versions of the
line-buffering loop stored *every* return value unconditionally,
including empty polls, which happen far more often than real
keystrokes arrive. Since nothing distinguished "real character" from
"nothing yet," the buffer filled with `UART_NO_DATA`-derived garbage
in microseconds, before the user finished typing even one key.

First fix attempt filtered by printable-ASCII range
(`c >= 32 && c <= 127`) — this technically worked (`-1` falls outside
the range) but conflated two different concerns: excluding the
sentinel vs. validating that input is printable. That range would
also silently swallow backspace, tab, and other control characters
later, as a side effect nobody asked for. Settled on the precise,
intention-matching check instead: `c != UART_NO_DATA`.

**Off-by-one in null-termination: postfix `++` writes at the *old*
index, not the new one.** `buffer[index++] = '\0';` looks like it
should "advance, then write," but postfix `++` returns the old value
of `index` for use in the expression and only updates it afterward —
so the write still lands on the last real character's slot, clobbering
it. Same root confusion showed up again, in a different shape, inside
`mini_strtok`'s scan loop: `*(buffer++) != ' '` bundles "read this
character" and "advance past it" into one expression, and the advance
fires as a side effect *during* the very comparison that ends the
loop — so the pointer overshoots the delimiter by one position before
the loop even registers it should stop. Fixed by separating the two
actions: check `*start` on its own (no increment folded into the
condition), and only `start++` inside the loop body once the check
confirms it's safe to keep going. That leaves `start` sitting exactly
on the delimiter/terminator when the loop exits, instead of one past it.

**A cast at the call site doesn't fix a type mismatch at the
callee.** While chasing an `int`/`char` mismatch between `main.c`'s
buffer and `mini_strtok`'s parameter, tried `mini_strtok((char *)buffer)`
as a fix. This doesn't work: once execution is inside `mini_strtok`,
its own declared parameter type governs how pointer arithmetic and
dereferencing behave (`buffer++` advances by `sizeof(int)` if the
parameter is declared `int *`, regardless of what type the caller
originally had or cast to at the call site). The real fix has to
happen at the parameter's actual declaration, anchored to what the
data *is* — a C string is always represented as `char`, independent
of whatever any other function happens to declare.

**Getting `strtok`'s `NULL`-continuation state machine right took
three iterations.** Implementing "pass a buffer once, then `NULL` to
resume" needs a `static` pointer to carry state between calls, since C
gives no other way for one call to hand context to the next without a
shared variable or explicit output parameter.
1. First pass never wrote back into `private_buffer` at all —
   state was computed locally and thrown away, so a later `NULL` call
   had nothing valid to resume from.
2. Second pass added the write, and correctly distinguished "found a
   token" from "nothing was there at all" via `start == toc` — but
   still unconditionally did the "consume delimiter, advance past it"
   step even when the scan stopped at the string's real terminator
   (as opposed to a space). That walked `private_buffer` one byte past
   the end of the valid string into uninitialized memory, so a
   *third* call (right after correctly returning the final token)
   would scan garbage instead of reporting "no more tokens."
3. Fixed by only performing the "terminate and advance past the
   delimiter" step when the scan actually stopped on `' '`. When it
   stopped on `'\0'` instead, `private_buffer` is left pointing
   directly at that terminator — so the next call's `start == toc`
   check fires immediately and correctly reports exhaustion.

### New concepts

**De Morgan's law as a practical loop-writing tool**, not just a
boolean-algebra fact: "stop if A or B" → "continue while (not A) and
(not B)." Worth reaching for explicitly any time a stop condition is
built from more than one check.

**Parameter types are the callee's contract with itself, not just
the caller's.** A cast at a call site can silence a compiler warning
about the argument, but it can't change how the function's own body
interprets a pointer once execution is inside it — that's governed
entirely by the parameter's declared type.

### Open / deferred (matches real `strtok`'s own limitations, or
judged low priority for now)

- `mini_strtok(NULL)` called before any prior call with a real buffer
  is undefined behavior (null-pointer dereference) — same footgun
  real `strtok` has.
- Leading delimiters aren't skipped: a string starting with `' '`
  returns "no token" immediately rather than skipping to the first
  real word, unlike real `strtok`. Doesn't matter for the current
  fixed-format line input; would matter if input parsing gets more
  freeform later.
