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
version-pinned and updated independently. They're already registered and
pinned to specific tested commits in this repo:

- `lib/pico-sdk` → tag `2.3.0`
- `lib/FreeRTOS-Kernel` → `main` branch, a commit confirmed to build cleanly
  against that pico-sdk version for both RP2040 and RP2350

You don't need to run `git submodule add` yourself — that's already done.
After cloning, fetch the top-level submodules and the one nested submodule
this project actually needs:

```bash
git clone <this-repo-url>
cd freertos_raspi
git submodule update --init                      # pico-sdk, FreeRTOS-Kernel
git -C lib/pico-sdk submodule update --init lib/tinyusb
```

`setup-host.sh` runs both of these for you automatically — this is what to
do if you're setting things up by hand instead, or need to re-fetch after
a reset.

**Don't use `git clone --recurse-submodules` or
`git submodule update --init --recursive`** — both pull in *every* nested
submodule of `pico-sdk`, including `lib/cyw43-driver`, `lib/lwip`,
`lib/mbedtls`, and `lib/btstack`. Those are only needed for Pico W
wireless/Bluetooth features, are large, and aren't used by the blink
example — the commands above fetch only what this project actually
builds with. If you later need Pico W wireless features, initialize the
specific submodule you need:

```bash
cd lib/pico-sdk
git submodule update --init lib/cyw43-driver   # example
```

If you ever want to bump either submodule to a newer upstream commit:

```bash
cd lib/pico-sdk        # or lib/FreeRTOS-Kernel
git fetch
git checkout <new-tag-or-commit>
cd ../..
git add lib/pico-sdk
git commit -m "Bump pico-sdk to <version>"
```

`setup-host.sh` also runs both fetch commands for you automatically on
subsequent resets.

## Environment variables

The build needs `PICO_SDK_PATH` and `FREERTOS_KERNEL_PATH`. `setup-host.sh`
appends these to `~/.bashrc`:

```bash
export PICO_SDK_PATH="$HOME/freertos_raspi/lib/pico-sdk"
export FREERTOS_KERNEL_PATH="$HOME/freertos_raspi/lib/FreeRTOS-Kernel"
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
