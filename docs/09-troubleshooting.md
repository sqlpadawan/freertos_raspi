# 09 — Troubleshooting

## `pico/stdlib.h: No such file or directory` when pressing an inline ▷ button

If this happens the moment you press a small ▷ (play) icon docked directly
above an open source file's tab — not the Run and Debug side panel — you
likely triggered **CMake Tools' own inline Run/Debug button**, which is a
separate feature from this project's actual debug setup. It builds and
runs using whatever kit CMake Tools currently has selected, and if that's
reverted to your Pi's native `gcc` instead of the `arm-none-eabi` kit
(easy to happen after a kit rescan or reopening the folder), the compile
fails immediately on the first pico-sdk header it hits, since native gcc
has no reason to know where `pico/stdlib.h` lives.

Fix: re-run `CMake: Select a Kit` from the Command Palette and confirm
the `arm-none-eabi` one is selected, then use the **Run and Debug side
panel** (`Ctrl+Shift+D`) and pick the correct launch configuration from
its dropdown instead of the inline button — see
[03-vscode-setup.md](03-vscode-setup.md#everyday-workflow-after-first-time-setup).

## `PICO_SDK_PATH` / `FREERTOS_KERNEL_PATH` not set

```
CMake Error: PICO_SDK_PATH not found
```

Run `source ~/.bashrc`, or re-run `scripts/setup-host.sh`, or check
`echo $PICO_SDK_PATH`. If the repo was cloned to a different path than
`~/freertos_raspi`, update the export lines in `~/.bashrc` manually.

## Submodules empty (`lib/pico-sdk` has no files)

```bash
git submodule update --init
git -C lib/pico-sdk submodule update --init lib/tinyusb
```

Don't use `--recursive` here — see
[02-host-toolchain-setup.md](02-host-toolchain-setup.md) for why (it pulls
in several large, unused Pico W submodules).

## Permission denied on `/dev/ttyACM0`

Your user needs to be in the `dialout` group:

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in (or reboot) for the group change to take effect.

## `code --install-extension` fails with "Extension ... not found"

The extension ID this repo uses (`raspberry-pi.raspberry-pi-pico`) is
correct — verified against the official marketplace listing. If you hit
"not found" anyway, it's almost always a transient marketplace lookup
issue (network hiccup, DNS, or a rate limit), not a wrong ID. The `|| true`
after each install line in `setup-host.sh` means this doesn't abort the
rest of the script — you'll still see "Setup complete." even if one
extension failed.

To resolve:

1. `code --version` — confirm VS Code itself is actually working.
2. `curl -I https://marketplace.visualstudio.com` — confirm the Pi can
   reach the marketplace. If this fails, fix connectivity/DNS first.
3. Just retry: `code --install-extension raspberry-pi.raspberry-pi-pico`
4. If the CLI keeps failing, try the GUI instead — Extensions panel
   (`Ctrl+Shift+X`) → search "Raspberry Pi Pico" — it sometimes succeeds
   when the CLI's gallery query doesn't.
5. Last resort: download the latest `.vsix` from the
   [pico-vscode releases page](https://github.com/raspberrypi/pico-vscode/releases)
   and install via `Ctrl+Shift+P` → "Extensions: Install from VSIX...".

## "You are trying to start Visual Studio Code as a super user"

This happens if `setup-host.sh` was invoked with `sudo` (e.g.
`sudo ./scripts/setup-host.sh`), which makes every command in the script —
including the `code --install-extension` calls at the end — inherit root
privileges. VS Code refuses to run as root and prints this warning
repeatedly instead of installing the extensions.

The script now guards against this and exits immediately with an error if
run as root. If you hit the warning, you're on an older copy — pull the
latest version of this repo, then re-run **without** `sudo`:

```bash
./scripts/setup-host.sh
```

The script escalates internally (via inline `sudo`) only for the specific
steps that actually need root — `apt-get`, `usermod`, and `make install`
for OpenOCD — so running it as your normal user is correct and expected;
you'll just get a password prompt at those specific points.

If extensions failed to install because of this, install them manually
once VS Code is present:

```bash
code --install-extension raspberry-pi.raspberry-pi-pico
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension marus25.cortex-debug
```

## OpenOCD build: `fatal: Remote branch rp2040 not found in upstream origin`

Raspberry Pi's OpenOCD fork used to publish a branch named `rp2040`; that
branch has since been retired. Their current default branch is
`rpi-common` and supports both RP2040 and RP2350, so `setup-host.sh` now
clones without specifying `--branch` at all (it just uses whatever the
default branch is). If you hit this error, you're running an older copy
of `setup-host.sh` — pull the latest version of this repo, or manually
re-run the clone step without `--branch rp2040`:

```bash
rm -rf ~/openocd-rpi-src
git clone https://github.com/raspberrypi/openocd.git ~/openocd-rpi-src
cd ~/openocd-rpi-src
git submodule update --init   # jimtcl
./bootstrap
./configure --enable-linuxgpiod --enable-cmsis-dap --enable-internal-jimtcl
make -j$(nproc)
sudo make install
```

> ⚠️ **WARNING — irreversible:** `rm -rf ~/openocd-rpi-src` deletes a
> previous (possibly partial) clone of the OpenOCD source tree. This is
> safe — it's just a scratch clone, not anything you authored — but as
> always, double check the path before running `rm -rf` with a fresh eye.

## OpenOCD: "Error: unable to open CMSIS-DAP device"

Applies to both the official Debug Probe and a DIY spare-Pico probe — they
present the same USB IDs since they run the same firmware.

- DIY probe: confirm `debugprobe.uf2` actually flashed (see
  [07-debugging-swd.md](07-debugging-swd.md#flashing-a-diy-probe-spare-pico-only)).
  Official Debug Probe: this step doesn't apply, it ships ready to use.
- Wrong USB cable (some are power-only). Try a different cable/port.
- Check `lsusb` shows the probe (`2e8a:000c` for CMSIS-DAP).
- On Raspberry Pi OS, you may need a udev rule:
  ```bash
  echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="2e8a", MODE="0666"' | \
    sudo tee /etc/udev/rules.d/99-debugprobe.rules
  sudo udevadm control --reload-rules && sudo udevadm trigger
  ```
  Unplug and replug the probe after adding the rule.

## OpenOCD: "Error: libgpiod: unable to request lines" (direct GPIO/B2 wiring)

- Another process (or a previous unclosed OpenOCD instance) is holding the
  GPIO lines. Check for stray processes:
  ```bash
  pgrep -a openocd
  ```
  Kill any leftover instance by PID: `kill <pid>` (avoid `kill -9` unless
  it's unresponsive — a plain `kill` lets it release GPIO lines cleanly).
- Confirm your user has GPIO access (`gpio` group) on Raspberry Pi OS.

## `CMake Error: PICO_PLATFORM is specified to be 'rp2040', but PICO_BOARD='pico2' ... incompatible`

You switched `PICO_BOARD` (e.g. `pico` → `pico2`) without clearing the
existing `build/` directory first. RP2040 and RP2350 are different chip
platforms, and pico-sdk refuses to reconfigure an existing cache across
that boundary rather than silently doing something wrong. Fix:

```bash
rm -rf build
cmake -B build -DPICO_BOARD=pico2 -G Ninja .
```

See [05-building.md](05-building.md#switching-the-target-board-pico--pico-2)
for the full explanation, including the separate-build-directory approach
if you switch between boards often.

## Build succeeds but board does nothing after flashing (LED not blinking)

Work through these in order — each one narrows down where the problem is:

1. **Confirm the UF2 copy actually completed.** When
   `cp build/src/blink.uf2 /media/$USER/RPI-RP2/` succeeds, the Pico
   automatically unmounts and reboots within a second or two. If the drive
   is still sitting there mounted, the copy likely didn't finish — retry it.

2. **Check serial output to see if the firmware is actually running:**
   ```bash
   ls /dev/ttyACM*
   minicom -D /dev/ttyACM0 -b 115200
   ```
   If you see `LED ON` / `LED OFF` printing every 500ms, the firmware
   **is** running correctly — the problem is isolated to the LED/GPIO
   itself (go to step 4). If you see nothing at all, go to step 3.

3. **Confirm `PICO_BOARD` matches your actual hardware:**
   ```bash
   grep PICO_BOARD build/CMakeCache.txt
   ```
   Flashing a `pico2` (RP2350) build onto an original Pico (RP2040), or
   vice versa, produces a UF2 that won't run correctly on the wrong
   silicon — no crash message, no LED, often no serial output either,
   since it may not get far enough to initialize USB.

4. **Do you have a Pico W or Pico 2 W (wireless variant)?** This is the
   most common cause of "firmware runs fine, LED just never turns on."
   On W boards, the onboard LED is **not** wired to a plain GPIO pin —
   it's connected through the CYW43439 wireless chip, and needs the
   `pico_cyw43_arch` library plus a small amount of extra init code to
   control (`cyw43_arch_init()` + `cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, ...)`
   instead of `gpio_put()`). This project's `main.c` and `CMakeLists.txt`
   currently target plain (non-W) boards only and use the regular
   `gpio_put(PICO_DEFAULT_LED_PIN, ...)` approach — on a W board the code
   still builds and runs (you'll see the serial output in step 2), it just
   silently can't reach the LED. Confirming this is your situation:
   check the board's silkscreen / packaging for a "W" designation, or
   look for the CYW43439 chip (small square IC with a small antenna
   pattern nearby) — if you don't have a wireless variant, this doesn't
   apply and you should keep looking elsewhere (loose LED, wrong board
   revision, etc.).

   If you do have a W board and want the LED working, you'll need to:
   add `pico_cyw43_arch_lwip_threadsafe_background` (or the `_poll`
   variant) to `target_link_libraries` in `src/CMakeLists.txt`, initialize
   it with `cyw43_arch_init()` before use, and swap the `gpio_*` calls in
   `src/main.c` for their `cyw43_arch_gpio_*` equivalents. This also pulls
   in the `lib/cyw43-driver` and `lib/lwip` pico-sdk submodules that
   [02-host-toolchain-setup.md](02-host-toolchain-setup.md) intentionally
   skips fetching by default.

5. **Device repeatedly fails to enumerate, then falls back to appearing as
   the bootloader (`RP2350 Boot` / `RP2 Boot`) again** — check with
   `dmesg | tail -40` right after plugging in. If you see repeated
   `device descriptor read/64, error -110` and `attempt power cycle`
   messages, followed by the device eventually re-appearing as the
   bootloader's mass-storage device rather than your application, this is
   the chip's own bootrom safety mechanism: your firmware is crashing or
   resetting very early — before it can bring up USB — and after a few
   failed attempts the bootrom falls back to presenting itself as a
   bootloader again rather than getting stuck. This is a genuine firmware
   problem, not something wrong with your flashing process, cable, or the
   `PICO_BOARD` setting (those are all already covered above).

   Before diving into SWD debugging, build and flash the diagnostic
   **bare-metal blink** target included in this repo — plain pico-sdk,
   no FreeRTOS at all:
   ```bash
   ninja -C build blink_bare
   cp build/src/blink_bare.uf2 /media/$USER/RPI-RP2/
   ```
   (or `RP2350` if that's what your board's mass-storage volume is named).
   Then repeat steps 1–2 above against this new binary.

   - **If `blink_bare` blinks and prints fine** — the problem is isolated
     to something in the FreeRTOS/RP2350 configuration specifically (not
     the board, cable, toolchain, or base pico-sdk setup). At that point,
     SWD debugging the *FreeRTOS* `blink` target (not `blink_bare`) with a
     breakpoint at the top of `main()` in `src/main.c` is the most direct
     way to see exactly where it stops — see
     [07-debugging-swd.md](07-debugging-swd.md).
   - **If `blink_bare` also fails the same way** — the problem is more
     fundamental than FreeRTOS: something in the toolchain, the board
     itself, or the wiring/power setup. Revisit steps 1–3 above with
     fresh eyes, and consider trying a different USB cable/port if you
     haven't already.

## Cortex-Debug shows no RTOS thread info

- Confirm `FreeRTOSConfig.h` has debug/RTOS-awareness symbols intact
  (don't strip debug info from the `.elf` used for debugging — flash the
  `.elf`, not the `.bin`, when debugging).
- Confirm the Cortex-Debug `rtos` field in `launch.json` is set to
  `"FreeRTOS"`.

## Still stuck?

Capture the exact error text and which doc step you were on — this repo's
docs are meant to be corrected/expanded as issues are found. Add new
entries to this file once resolved so the next reset doesn't hit the same
wall.
