# Bare-Metal ARM Cortex-M3 — TI LM3S6965EVB

A bare-metal C firmware project targeting the TI LM3S6965 microcontroller,
emulated in QEMU. No vendor HALs, no CMSIS, no dynamic allocation — everything
is written from scratch using the LM3S6965 datasheet and the ARMv7-M
Architecture Reference Manual.

This is a learning project, working through the full stack from reset vector
to a cooperative task scheduler.

## Environment

- **Host:** macOS
- **Cross-compiler:** `arm-none-eabi-gcc` (via Homebrew)
- **Emulator:** `qemu-system-arm` (`lm3s6965evb` machine)
- **Debugger:** `arm-none-eabi-gdb` / LLDB
- **Build system:** GNU Make (hand-written)

No physical hardware — purely software.

## Build

```bash
make
```

Output: `build/main.elf`

## Run

```bash
./run.sh
```

Launches QEMU with the compiled ELF. Interrupt with `Ctrl-C` to stop.

## Inspect

```bash
./debug.sh
```

Runs `arm-none-eabi-objdump -h` on the compiled ELF to display section
headers — useful for verifying that sections landed at the correct VMA/LMA
addresses.

## Roadmap

- [x] Linker script, startup code, vector table, reset handler
- [X] GPIO — toggle an LED via direct register writes
- [X] UART driver and mini printf
- [X] Interrupts — SysTick, NVIC, ring buffers
- [ ] UART command shell
- [ ] Cooperative / preemptive task scheduler
