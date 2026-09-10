# Pico FreeRTOS Dev Setup (Raspberry Pi 5 Host)

Reproducible setup for developing **FreeRTOS** applications for the
**Raspberry Pi Pico / Pico 2** (RP2040 / RP2350), using a **Raspberry Pi 5**
running **Raspberry Pi OS** as the development machine, with **VS Code** as
the IDE.

This repo is designed to be cloned onto a fresh Raspberry Pi OS install and
bring you from "blank SD card" to "blinking LED under FreeRTOS, with
breakpoint debugging" by following the docs in order.

> **Known issue:** on a **Pico 2 (RP2350)**, the FreeRTOS blink example
> currently hangs and never brings up USB serial — this is a known,
> currently-unresolved upstream issue with FreeRTOS + USB CDC on RP2350,
> not something specific to this repo's setup. The original **Pico
> (RP2040)** is not affected. See
> [docs/10-known-issue-rp2350-freertos-usb.md](docs/10-known-issue-rp2350-freertos-usb.md)
> for the full writeup before spending time debugging this yourself on a
> Pico 2.

## Why this repo exists

Every step of the host setup, toolchain install, VS Code configuration, and
hardware wiring is documented in [`docs/`](docs/) so the whole environment
can be **torn down and rebuilt from scratch** at any time — no tribal
knowledge, no undocumented manual steps.

## Hardware

| Item | Role |
|---|---|
| Raspberry Pi 5 (running Raspberry Pi OS) | Development host — runs VS Code, toolchain, OpenOCD |
| Raspberry Pi Pico or Pico 2 | FreeRTOS target (RP2040 or RP2350) |
| USB-A/C to Micro-USB/USB-C cable | Power + UF2 flashing |
| (Recommended) [Raspberry Pi Debug Probe](https://www.raspberrypi.com/products/debug-probe/) | SWD + UART debugging, no flashing required |
| (Alternative) second Pico as DIY probe, or 3 jumper wires to Pi 5 GPIO | SWD debugging without buying the official probe |

See [`docs/01-hardware-setup.md`](docs/01-hardware-setup.md) for wiring details.

## Quick start

```bash
git clone <this-repo-url>
cd freertos_raspi
./scripts/setup-host.sh          # do NOT run with sudo — it fetches submodules, installs the toolchain, and escalates internally where needed
code .                           # open in VS Code
```

Then follow the docs in order:

1. [Hardware setup](docs/01-hardware-setup.md)
2. [Host toolchain setup](docs/02-host-toolchain-setup.md)
3. [VS Code configuration](docs/03-vscode-setup.md)
4. [Project structure](docs/04-project-structure.md)
5. [Building](docs/05-building.md)
6. [Flashing (UF2)](docs/06-flashing-uf2.md)
7. [Debugging (SWD)](docs/07-debugging-swd.md)
8. [FreeRTOS configuration](docs/08-freertos-config.md)
9. [Troubleshooting](docs/09-troubleshooting.md)
10. [Known issue: FreeRTOS + USB on Pico 2](docs/10-known-issue-rp2350-freertos-usb.md)
11. [Wi-Fi networking (Pico W / Pico 2 W)](docs/11-wifi-networking.md)

## Repo layout

```
.
├── docs/                   # All setup & config documentation (this is the source of truth)
├── src/                    # FreeRTOS application source (blink example)
├── .vscode/                # Editor tasks, launch configs, recommended extensions
├── scripts/                 # Host setup / bootstrap scripts
├── lib/                    # Git submodules: pico-sdk, FreeRTOS-Kernel (added after first setup)
├── FreeRTOSConfig.h        # FreeRTOS kernel configuration for this project
├── CMakeLists.txt          # Top-level build config
└── pico_sdk_import.cmake   # Standard Pico SDK import shim
```

## Status

Verified on **real Pico 2 (RP2350) hardware**:
- [x] Docs scaffolded
- [x] `blink` (FreeRTOS) and `blink_bare` (no RTOS) both build cleanly
- [x] UF2 flashing
- [x] `blink_bare` — LED blinks, USB serial prints correctly
- [x] Debug Probe / SWD connection (OpenOCD connects cleanly, both
      Cortex-M33 cores detected)
- [ ] `blink` (FreeRTOS) — **known issue**, hangs before USB comes up; see
      [docs/10-known-issue-rp2350-freertos-usb.md](docs/10-known-issue-rp2350-freertos-usb.md)

Not yet verified on **real Pico (RP2040) hardware** (build-only, in a
sandbox with no physical board) — expected to work based on the FreeRTOS
RP2040 port being mature and widely used, but not yet confirmed
end-to-end on this project specifically:
- [ ] UF2 flashing
- [ ] `blink` (FreeRTOS) — LED + USB serial
- [ ] Debug Probe / SWD connection

Other:
- [ ] Second example (queue/semaphore) added

## License

MIT — see [`LICENSE`](LICENSE).
