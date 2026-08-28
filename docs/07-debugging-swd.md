# 07 — Debugging via SWD

Once UF2 flashing works (see [06](06-flashing-uf2.md)), set this up to get
breakpoints, step-through debugging, and live variable/task inspection of
FreeRTOS in VS Code.

Pick **one** of the two wiring options from
[01-hardware-setup.md](01-hardware-setup.md); both use the same OpenOCD
config below, just a different `interface` line.

## OpenOCD config

Two interface config files are used depending on wiring:

- **Picoprobe (second Pico):** `interface/cmsis-dap.cfg`
- **Direct Pi 5 GPIO:** `interface/linuxgpiod.cfg`

Target config is the same either way:

- RP2040 (original Pico): `target/rp2040.cfg`
- RP2350 (Pico 2): `target/rp2350.cfg`

### Manual OpenOCD invocation (for testing the connection)

```bash
# Picoprobe:
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

`.vscode/launch.json` in this repo already points Cortex-Debug at OpenOCD
and `gdb-multiarch`. Set the `interface` field to match your wiring choice,
then:

1. Set a breakpoint in `src/main.c` (e.g. inside the blink task's loop)
2. `Run and Debug` → select the configured launch target
3. Execution should halt at the breakpoint, with FreeRTOS task state
   visible in the Cortex-Debug "RTOS Threads" panel

## A note on flashing via OpenOCD instead of UF2

Once SWD works, you can flash without touching BOOTSEL at all:

```bash
openocd -f interface/<yours>.cfg -f target/rp2040.cfg \
  -c "program build/src/blink.elf verify reset exit"
```

This is what the VS Code "Flash" task and the debug launch config use
under the hood.
