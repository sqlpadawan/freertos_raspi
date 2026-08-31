# 07 — Debugging via SWD

Once UF2 flashing works (see [06](06-flashing-uf2.md)), set this up to get
breakpoints, step-through debugging, and live variable/task inspection of
FreeRTOS in VS Code.

Pick **one** of the three wiring options from
[01-hardware-setup.md](01-hardware-setup.md). All three use the same
underlying OpenOCD target config, just a different `interface` line —
and the official Debug Probe and the DIY spare-Pico probe use the
*identical* interface config, since they run the same firmware.

| Wiring option | Interface config |
|---|---|
| Debug Probe (official) or DIY probe (spare Pico) | `interface/cmsis-dap.cfg` |
| Direct Pi 5 GPIO | `interface/linuxgpiod.cfg` |

Target config is the same for every option:

- RP2040 (original Pico): `target/rp2040.cfg`
- RP2350 (Pico 2): `target/rp2350.cfg`

This repo's `.vscode/launch.json` and `tasks.json` default to the Debug
Probe / DIY-probe path (`cmsis-dap.cfg`) since it's the simplest to get
working reliably. Switch to `linuxgpiod.cfg` there if you're using Option C.

## Flashing a DIY probe (spare Pico only)

Skip this section entirely if you bought the official Debug Probe — it
ships pre-flashed and needs nothing done to it.

If you're turning a spare Pico into a probe instead:

1. Download the latest `debugprobe.uf2` from the
   [raspberrypi/debugprobe releases page](https://github.com/raspberrypi/debugprobe/releases)
   (grab the `debugprobe_on_pico.uf2` variant for a plain Pico).
2. Hold BOOTSEL on the spare Pico, plug it into the Pi 5, release BOOTSEL.
3. Copy the file onto it, same as any UF2:
   ```bash
   cp debugprobe_on_pico.uf2 /media/$USER/RPI-RP2/
   ```
4. It reboots running probe firmware and will enumerate as a CMSIS-DAP
   device from then on — verify with:
   ```bash
   lsusb | grep -i "2e8a:000c"
   ```

## Manual OpenOCD invocation (for testing the connection)

```bash
# Debug Probe (official or DIY):
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg

# Direct GPIO:
openocd -f interface/linuxgpiod.cfg -f target/rp2040.cfg
```

A successful connection prints something like:

```
Info : SWD DPIDR 0x0bc12477
Info : rp2040.core0: hardware has 4 breakpoints, 2 watchpoints
```

If it hangs or errors out, see [09-troubleshooting.md](09-troubleshooting.md)
before touching wiring again.

## Connecting GDB manually: a `vMustReplyEmpty` compatibility quirk

If you connect GDB to OpenOCD by hand (`gdb-multiarch your.elf` then
`target remote localhost:3333`), you may hit:

```
Remote replied unexpectedly to 'vMustReplyEmpty': vCont;c;C;s;S
```

This is a long-documented compatibility quirk between modern GDB versions
and OpenOCD's gdbserver implementation (OpenOCD's `qSupported` response
includes a nonstandard `"timeout"` item that confuses GDB's next
handshake step) — it's not specific to this project, this board, or your
setup, and shows up across many different targets (STM32, ESP32, RISC-V)
in public bug trackers going back years.

**What we confirmed works, manually, outside VS Code:** using ARM's
official toolchain's `arm-none-eabi-gdb` (not Debian's `gdb-multiarch`)
together with `target extended-remote` instead of `target remote`.

**What's still unconfirmed:** whether this fully resolves the issue
*inside VS Code's Cortex-Debug integration* — Cortex-Debug already
connects via `extended-remote` internally, and an early attempt through
VS Code hit this same error (though at the time, `launch.json` also had
the wrong `target/rp2350.cfg` value, confounding that result). If you hit
this through VS Code specifically, try changing `gdbPath` in
`launch.json` from `gdb-multiarch` to the full path of ARM's
`arm-none-eabi-gdb` (see [02-host-toolchain-setup.md](02-host-toolchain-setup.md)
if you need to install it) and retest — this is a reasonable next step,
not a verified fix.

## VS Code integration

`.vscode/launch.json` in this repo defines four configurations — pick
whichever matches your **board and wiring** in the "Run and Debug"
dropdown:

- **"Debug Pico (RP2040) — Debug Probe / SWD"** — original Pico, official
  Debug Probe or DIY spare-Pico probe (both use `cmsis-dap.cfg`)
- **"Debug Pico 2 (RP2350) — Debug Probe / SWD"** — Pico 2, same probe options
- **"Debug Pico (RP2040) — Direct Pi GPIO / SWD"** — original Pico, Option C wiring
- **"Debug Pico 2 (RP2350) — Direct Pi GPIO / SWD"** — Pico 2, Option C wiring

Picking the wrong board's configuration won't necessarily fail loudly —
OpenOCD may still connect, since both chips implement standard SWD, but
target-specific operations (like the RTOS-awareness the Cortex-Debug
integration relies on) can behave oddly if the target config doesn't
match the actual silicon. Match this to whichever board you're building
for (see [05-building.md](05-building.md#switching-the-target-board-pico--pico-2)
if you're not sure which is currently configured).

Steps:

1. Set a breakpoint in `src/main.c` (e.g. inside the blink task's loop)
2. `Run and Debug` → select the configuration matching your board and wiring
3. Execution should halt at the breakpoint, with FreeRTOS task state
   visible in the Cortex-Debug "RTOS Threads" panel

## Serial console over the Debug Probe's UART cable

If you wired up the Debug Probe's second JST-SH cable to the target's
UART0 pins (see [01-hardware-setup.md](01-hardware-setup.md)), the probe
enumerates a *second* serial device (distinct from the target's own USB
CDC port used by `pico_enable_stdio_usb`):

```bash
ls /dev/ttyACM*
# e.g. /dev/ttyACM0 = target's own USB CDC (printf via pico_stdlib)
#      /dev/ttyACM1 = Debug Probe's UART pass-through
minicom -D /dev/ttyACM1 -b 115200
```

This is useful once you're debugging over SWD and want console output on a
completely independent channel from the target's main USB connection —
handy if you ever have the target's USB port occupied by something else,
or want a console that survives a target reset via OpenOCD.

## A note on flashing via OpenOCD instead of UF2

Once SWD works, you can flash without touching BOOTSEL at all:

```bash
openocd -f interface/<yours>.cfg -f target/rp2040.cfg \
  -c "program build/src/blink.elf verify reset exit"
```

This is what the VS Code "Flash" task and the debug launch config use
under the hood.
