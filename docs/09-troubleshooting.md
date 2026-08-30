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
git submodule update --init --recursive
```

## Permission denied on `/dev/ttyACM0`

Your user needs to be in the `dialout` group:

```bash
sudo usermod -a -G dialout $USER
```

Log out and back in (or reboot) for the group change to take effect.

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
