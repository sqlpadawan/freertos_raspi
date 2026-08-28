# 01 — Hardware Setup

## What you need

| Item | Notes |
|---|---|
| Raspberry Pi 5 | Host machine, running Raspberry Pi OS (64-bit recommended) |
| Raspberry Pi Pico or Pico 2 | The FreeRTOS target. Pico = RP2040, Pico 2 = RP2350 |
| Micro-USB (Pico) or USB-C (Pico 2) cable | Power + flashing via UF2 bootloader |
| microSD card / SSD for the Pi 5 | Raspberry Pi OS install media |
| **Optional:** second Pico, or a Raspberry Pi Debug Probe | For SWD debugging (breakpoints, live variable inspection) |
| **Optional:** 3x female-female jumper wires | For wiring SWD directly from Pi 5 GPIO to target Pico |

## Two ways to get from Pi 5 → Pico

### Option A — USB only (start here)

Just plug the Pico into the Pi 5 via USB. This is enough to:
- Power the board
- Flash `.uf2` files by drag-and-drop or `picotool`
- Get serial output over USB CDC (`printf` debugging)

No wiring required. This is the fastest path to a working "blink" project.
See [06-flashing-uf2.md](06-flashing-uf2.md).

### Option B — Add SWD for real debugging (recommended once blink works)

To set breakpoints and step through FreeRTOS tasks in VS Code, you need an
SWD connection. Two ways to do this, both fully supported by the Raspberry
Pi Foundation tooling:

**B1. Second Pico as "Picoprobe"**
Flash a spare Pico with the `debugprobe` firmware; it acts as a USB-to-SWD
+ UART bridge. Wiring between the two Picos:

| Picoprobe (probe) | Target Pico | Signal |
|---|---|---|
| GP2 | SWCLK | Clock |
| GP3 | SWDIO | Data |
| GND | GND | Ground |
| GP4 (UART TX) | GP1 (UART RX) | Optional serial console |
| GP5 (UART RX) | GP0 (UART TX) | Optional serial console |

**B2. Direct from Raspberry Pi 5 GPIO (no second Pico needed)**
The Pi 5's GPIO header can bit-bang SWD directly via OpenOCD's `linuxgpiod`
driver — since your host *is* a Raspberry Pi, you don't strictly need a
separate probe.

| Pi 5 GPIO (BCM) | Pin # | Target Pico |
|---|---|---|
| GPIO 24 | Pin 18 | SWCLK |
| GPIO 25 | Pin 22 | SWDIO |
| GND | Pin 20 (any GND) | GND |

> ⚠️ **Double-check pin numbers before wiring.** Miswiring GPIO to 5V or
> reversing SWCLK/SWDIO won't usually damage anything on these boards, but
> confirm against the current [Pico datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)
> and your specific Pi 5 header revision before connecting power.

Both options are documented in detail in
[07-debugging-swd.md](07-debugging-swd.md) — you don't need to decide now.
The blink example in this repo works over plain USB with no wiring at all.

## Board identification

Run this after installing the toolchain (see next doc) to confirm the Pi 5
sees the Pico in bootloader mode:

```bash
lsusb | grep -i "2e8a"
```

`2e8a` is the Raspberry Pi Foundation USB vendor ID (used by both the Pico
bootloader and Picoprobe).
