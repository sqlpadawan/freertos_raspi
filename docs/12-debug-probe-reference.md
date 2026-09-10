# 12 — Debug Probe Reference: RTT and Firmware Updates

This doc covers two things from
[Raspberry Pi's official Debug Probe documentation](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html)
not covered elsewhere in this repo: RTT (an alternative to USB/UART for
`printf` output) and updating the Debug Probe's own firmware.

## RTT: `printf` output with no USB or UART at all

RTT (Real-Time Transport, a mechanism originally from Segger) sends your
program's output directly over the SWD connection — no USB CDC, no UART
wiring, nothing but the Debug Probe you already have connected for
debugging. It's a genuinely different code path from
`pico_enable_stdio_usb` and `pico_enable_stdio_uart`.

**Why this might matter for this project specifically:** the
[known RP2350 + FreeRTOS + USB CDC hang](10-known-issue-rp2350-freertos-usb.md)
is isolated to tinyusb's USB code specifically — not to FreeRTOS, not to
the RP2350 chip broadly, not to `printf` itself. RTT output never touches
USB at all. **This is an untried, plausible workaround for that issue,
not a confirmed fix** — we haven't tested it. If the hang really is
confined to tinyusb, switching `blink`'s stdio to RTT instead of USB CDC
could plausibly let a FreeRTOS + Wi-Fi-adjacent build run correctly on
Pico 2 even though USB serial doesn't. Worth trying before assuming
RP2350 is a dead end for anything beyond `blink_bare`.

### Enabling RTT in a target

In `src/CMakeLists.txt`, add (alongside the existing `pico_enable_stdio_usb`
line, or instead of it):

```cmake
pico_enable_stdio_rtt(blink 1)
```

**You must build as `Debug`, not `Release`, for RTT output to work** —
same requirement as [SWD debugging in general](07-debugging-swd.md#build-type-debug-not-release).

### Viewing RTT output in VS Code

Add an `rttConfig` block to the relevant configuration(s) in
`.vscode/launch.json`:

```json
"rttConfig": {
    "enabled": true,
    "address": "auto",
    "decoders": [
        {
            "label": "",
            "port": 0,
            "type": "console"
        }
    ]
}
```

Start a debug session as usual (see [07-debugging-swd.md](07-debugging-swd.md)).
Open the **TERMINAL** tab in VS Code — a new entry, `RTT Ch:0 console`,
appears in the dropdown on the right. Select it to see your `printf`
output.

### Viewing RTT output from a standalone (non-VS Code) session

With OpenOCD and GDB already connected manually (see
[07-debugging-swd.md](07-debugging-swd.md#manual-gdb-session-outside-vs-code)),
run these at the `(gdb)` prompt instead of a plain `continue`:

```
monitor reset init
monitor rtt setup 0x20000000 2048 "SEGGER RTT"
monitor rtt start
monitor rtt server start 60000 0
continue
```

Then, in a third terminal:

```bash
nc localhost 60000
```

This shows the RTT output live. `0x20000000` is a starting address for
OpenOCD to search from — it's the base of SRAM on both RP2040 and
RP2350, so this value doesn't need to change between boards.

## Updating the Debug Probe's firmware

The Debug Probe runs its own firmware (separate entirely from whatever
you flash onto the target Pico). Raspberry Pi occasionally releases
updates to it.

### Check your current version

```bash
lsusb -v -d 2e8a:000c | grep bcdDevice
```

This prints something like `bcdDevice 2.31`, meaning firmware version
2.3.1. As of when this doc was written, the latest published version is
2.3.1 — check the
[debugprobe releases page](https://github.com/raspberrypi/debugprobe/releases/latest)
for anything newer.

Note: OpenOCD's own connection log shows a line like
`Info : CMSIS-DAP: FW Version = 2.0.0` — that's the CMSIS-DAP *protocol*
version, a different number from the Debug Probe's own firmware version
above. Don't confuse the two.

### Updating

Same UF2 process as flashing any Pico target — because the Debug Probe
*is* a Pico-family chip running different firmware:

1. Download `debugprobe.uf2` from the
   [latest release](https://github.com/raspberrypi/debugprobe/releases/latest).
2. Hold BOOTSEL on the Debug Probe itself (not the target board), plug
   it into the Pi 5, release BOOTSEL — it mounts as `RPI-RP2` or
   `RP2350` depending on which Debug Probe hardware revision you have.
3. Copy the file onto it:
   ```bash
   cp debugprobe.uf2 /media/$USER/RPI-RP2/
   ```
4. It reboots automatically on completion, running the updated firmware.

If your probe reports an old version — Raspberry Pi's own docs
specifically call out anything older than 2.3.1, and the OpenOCD
connection log itself prints a warning
(`*** Old Raspberry Pi Debugprobe firmware detected (X.Y.Z)`) when this
is the case — it's worth updating before troubleshooting anything else
SWD-related, since older firmware is a real, if uncommon, source of
otherwise-confusing connection issues.
