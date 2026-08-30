# Pico FreeRTOS Dev Setup (Raspberry Pi 5 Host)

Reproducible setup for developing **FreeRTOS** applications for the
**Raspberry Pi Pico / Pico 2** (RP2040 / RP2350), using a **Raspberry Pi 5**
running **Raspberry Pi OS** as the development machine, with **VS Code** as
the IDE.

This repo is designed to be cloned onto a fresh Raspberry Pi OS install and
bring you from "blank SD card" to "blinking LED under FreeRTOS, with
breakpoint debugging" by following the docs in order.

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
git clone --recurse-submodules <this-repo-url>
cd pico-freertos
./scripts/setup-host.sh          # installs toolchain, SDKs, VS Code (if needed), and extensions
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

- [x] Docs scaffolded
- [x] Blink example (FreeRTOS task toggling onboard LED)
- [x] Debug Probe / SWD debugging documented (official probe, DIY probe, and direct-GPIO alternatives)
- [ ] SWD debug probe wired and verified on actual hardware
- [ ] Second example (queue/semaphore) added

## License

MIT — see [`LICENSE`](LICENSE).
