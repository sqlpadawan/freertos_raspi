# 09 — Troubleshooting

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

## Build succeeds but board does nothing after flashing

- Confirm `PICO_BOARD` in your CMake configure step matches your actual
  hardware (`pico` vs `pico2`) — wrong board setting is the most common
  cause of a "silent" flash.
- Check serial output (see [06-flashing-uf2.md](06-flashing-uf2.md)) for a
  crash/panic message before assuming it's a hardware fault.

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
