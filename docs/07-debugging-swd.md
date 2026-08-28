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

## VS Code integration

`.vscode/launch.json` in this repo already defines two configurations —
pick whichever matches your wiring in the "Run and Debug" dropdown:

- **"Debug (Debug Probe / SWD)"** — for the official Debug Probe or a
  DIY spare-Pico probe (both use `cmsis-dap.cfg`)
- **"Debug (Direct Pi GPIO / SWD)"** — for Option C wiring

Steps:

1. Set a breakpoint in `src/main.c` (e.g. inside the blink task's loop)
2. `Run and Debug` → select the configured launch target
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
