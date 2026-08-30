# 02 — Host Toolchain Setup (Raspberry Pi OS)

Everything here is automated by [`scripts/setup-host.sh`](../scripts/setup-host.sh).
This doc explains what that script does and why, so the setup isn't a black box.

## Packages installed

| Package | Purpose |
|---|---|
| `cmake`, `ninja-build` | Build system |
| `gcc-arm-none-eabi`, `libnewlib-arm-none-eabi` | ARM cross-compiler toolchain |
| `build-essential` | Host-side compilation (some SDK tools build native binaries) |
| `git` | Submodule management |
| `python3` | Required by Pico SDK build scripts |
| `libstdc++-arm-none-eabi-newlib` | C++ standard library for the target |
| `openocd` (built from source, see below) | On-target flashing/debugging via SWD |
| `gdb-multiarch` | Source-level debugging from VS Code |
| `minicom` or `screen` | Optional: manual serial console access |
| `code` (VS Code) | The IDE itself — installed automatically if not already present |

## Why OpenOCD is built from source

The Raspberry Pi Foundation maintains a fork of OpenOCD with RP2040/RP2350
support and the `linuxgpiod` adapter driver needed for direct Pi-GPIO SWD
(see [01-hardware-setup.md](01-hardware-setup.md) Option B2). The Debian
repo version is often too old. `setup-host.sh` clones and builds
`raspberrypi/openocd`.

## SDKs pulled in as submodules

| Path | Repo | Purpose |
|---|---|---|
| `lib/pico-sdk` | `raspberrypi/pico-sdk` | Board support, HAL, build glue |
| `lib/FreeRTOS-Kernel` | `raspberrypi/FreeRTOS-Kernel` (RP2040/RP2350 port) | The RTOS itself |

These are added as git submodules rather than vendored so they can be
version-pinned and updated independently. `.gitmodules` in this repo already
declares both, but since this scaffold wasn't cloned from a live GitHub repo
yet, you need to actually register them once, from the repo root, after your
first `git init`/push:

```bash
git submodule add https://github.com/raspberrypi/pico-sdk.git lib/pico-sdk
git submodule add https://github.com/raspberrypi/FreeRTOS-Kernel.git lib/FreeRTOS-Kernel
git submodule update --init --recursive
git add .gitmodules lib
git commit -m "Pin pico-sdk and FreeRTOS-Kernel submodules"
```

After that, anyone (including future-you, after a reset) just needs:

```bash
git clone --recurse-submodules <this-repo-url>
```

`setup-host.sh` also runs `git submodule update --init --recursive` for you
on subsequent resets, once the submodules are registered.

## Environment variables

The build needs `PICO_SDK_PATH` and `FREERTOS_KERNEL_PATH`. `setup-host.sh`
appends these to `~/.bashrc`:

```bash
export PICO_SDK_PATH="$HOME/pico-freertos/lib/pico-sdk"
export FREERTOS_KERNEL_PATH="$HOME/pico-freertos/lib/FreeRTOS-Kernel"
```

Restart your terminal (or `source ~/.bashrc`) after running the script.

## Verifying the install

```bash
arm-none-eabi-gcc --version
cmake --version
openocd --version
echo $PICO_SDK_PATH
echo $FREERTOS_KERNEL_PATH
```

All five should print sensible output with no errors. If any are blank or
missing, see [09-troubleshooting.md](09-troubleshooting.md).

## Installing VS Code

`setup-host.sh` installs VS Code automatically if the `code` command isn't
already on your PATH. Raspberry Pi OS (Bookworm and later) ships VS Code
directly in its own apt repos for arm64/armhf, so this is normally just:

```bash
sudo apt-get install -y code
```

If that package isn't available (older Raspberry Pi OS release, or a
non-standard image), the script falls back to adding Microsoft's official
apt repo and installing from there instead — no manual steps needed either
way. You only need to install VS Code yourself if you're running the docs
on a machine where `setup-host.sh` hasn't been run.

## Running the setup script

```bash
chmod +x scripts/setup-host.sh
./scripts/setup-host.sh
```

The script is idempotent — safe to re-run if a step fails partway through.
It only ever installs/builds; it does not delete or overwrite existing
config files without asking first.
