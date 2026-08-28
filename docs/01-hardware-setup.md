# 01 — Hardware Setup

## What you need

| Item | Notes |
|---|---|
| Raspberry Pi 5 | Host machine, running Raspberry Pi OS (64-bit recommended) |
| Raspberry Pi Pico or Pico 2 | The FreeRTOS target. Pico = RP2040, Pico 2 = RP2350 |
| Micro-USB (Pico) or USB-C (Pico 2) cable | Power + flashing via UF2 bootloader |
| microSD card / SSD for the Pi 5 | Raspberry Pi OS install media |
| **Recommended:** [Raspberry Pi Debug Probe](https://www.raspberrypi.com/products/debug-probe/) | Official USB-to-SWD/UART dongle — plug-and-play SWD debugging |
| **Alternative:** a second Pico | Can be flashed to act as a DIY debug probe (same firmware as above) |
| **Alternative:** 3x female-female jumper wires | For wiring SWD directly from Pi 5 GPIO to target Pico, no probe hardware at all |

## Three ways to get from Pi 5 → Pico

### Option A — USB only (start here)

Just plug the Pico into the Pi 5 via USB. This is enough to:
- Power the board
- Flash `.uf2` files by drag-and-drop or `picotool`
- Get serial output over USB CDC (`printf` debugging)

No wiring required. This is the fastest path to a working "blink" project.
See [06-flashing-uf2.md](06-flashing-uf2.md).

### Option B — Debug Probe (recommended once blink works)

To set breakpoints and step through FreeRTOS tasks in VS Code, you need an
SWD connection. The cleanest way to get one is the **official Raspberry Pi
Debug Probe** — a small USB-C dongle that speaks CMSIS-DAP and needs no
firmware flashing of its own.

It connects to the target Pico with its bundled 3-pin JST-SH cable:

| Debug Probe cable pin | Target Pico pin | Signal |
|---|---|---|
| Pin 1 (yellow) | SWCLK (pin next to SWDIO, near BOOTSEL end) | Clock |
| Pin 2 (black) | GND | Ground |
| Pin 3 (orange) | SWDIO | Data |

If your target Pico doesn't have a JST-SH debug connector broken out
(original Pico doesn't; Pico 2 and Pico W/2 W boards vary), solder or clip
onto the corresponding `SWCLK`/`GND`/`SWDIO` test points instead — see the
[Pico datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
pinout diagram.

A second JST-SH cable on the Debug Probe carries UART (target serial
console) — connect it to the target's UART0 TX/RX/GND pins if you want a
serial console independent of the USB CDC port.

**No firmware to flash** — plug the Debug Probe into the Pi 5 via USB-C and
it's immediately usable by OpenOCD. See
[07-debugging-swd.md](07-debugging-swd.md) for the OpenOCD config and VS Code
integration.

**DIY alternative:** if you don't have the official probe, a spare Pico can
be flashed with the same `debugprobe` firmware Raspberry Pi ships on the
retail unit, making it electrically and functionally identical. Wiring
between the two Picos:

| DIY probe (spare Pico) | Target Pico | Signal |
|---|---|---|
| GP2 | SWCLK | Clock |
| GP3 | SWDIO | Data |
| GND | GND | Ground |
| GP4 (UART TX) | GP1 (UART RX) | Optional serial console |
| GP5 (UART RX) | GP0 (UART TX) | Optional serial console |

Flashing instructions for the DIY probe are in
[07-debugging-swd.md](07-debugging-swd.md#flashing-a-diy-probe-spare-pico-only).

### Option C — Direct from Raspberry Pi 5 GPIO (no probe hardware at all)

The Pi 5's GPIO header can bit-bang SWD directly via OpenOCD's `linuxgpiod`
driver — since your host *is* a Raspberry Pi, you don't strictly need any
separate probe, official or DIY.

| Pi 5 GPIO (BCM) | Pin # | Target Pico |
|---|---|---|
| GPIO 24 | Pin 18 | SWCLK |
| GPIO 25 | Pin 22 | SWDIO |
| GND | Pin 20 (any GND) | GND |

> ⚠️ **Double-check pin numbers before wiring.** Miswiring GPIO to 5V or
> reversing SWCLK/SWDIO won't usually damage anything on these boards, but
> confirm against the current [Pico datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
> and your specific Pi 5 header revision before connecting power.

All three options are documented in detail in
[07-debugging-swd.md](07-debugging-swd.md) — you don't need to decide now.
The blink example in this repo works over plain USB with no wiring at all,
and the Debug Probe path (Option B) is the one this repo's default VS Code
debug config targets.

## Board identification

Run this after installing the toolchain (see next doc) to confirm the Pi 5
sees the Pico in bootloader mode:

```bash
lsusb | grep -i "2e8a"
```

`2e8a` is the Raspberry Pi Foundation USB vendor ID (used by both the Pico
bootloader and Picoprobe).
