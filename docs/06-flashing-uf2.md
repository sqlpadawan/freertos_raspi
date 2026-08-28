# 06 — Flashing via UF2 (no debug probe required)

This is the easiest way to get code onto the Pico and the right place to
start before setting up SWD debugging.

## Steps

1. Hold the **BOOTSEL** button on the Pico.
2. Plug it into the Pi 5 via USB while still holding BOOTSEL.
3. Release BOOTSEL. The Pico mounts as a USB mass-storage drive named
   `RPI-RP2` (Pico) or `RP2350` (Pico 2).
4. Copy the built UF2 file onto it:

   ```bash
   cp build/src/blink.uf2 /media/$USER/RPI-RP2/
   ```

   (Adjust the mount path — check `lsblk` or your file manager if it
   differs.)
5. The Pico automatically unmounts, resets, and starts running the new
   firmware. The onboard LED should start blinking.

## Using `picotool` instead (optional, no BOOTSEL button press needed)

`picotool` can reboot an already-running Pico into bootloader mode over
USB, which is handy once you no longer have physical access to the button:

```bash
picotool load build/src/blink.uf2 -f
```

The `-f` flag forces the board into BOOTSEL mode first if it's currently
running application code.

## Serial output

With the blink example running, view `printf` output over USB serial:

```bash
minicom -D /dev/ttyACM0 -b 115200
```

(Exit minicom with `Ctrl+A` then `X`.) The device path may be `/dev/ttyACM1`
etc. if other serial devices are attached — check `ls /dev/ttyACM*`.
