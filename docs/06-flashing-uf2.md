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

With the blink example running, view `printf` output over USB serial.

### Find all connected serial devices

```bash
ls /dev/ttyACM*
```

If you only have the target board plugged in, you'll likely see exactly
one device (`/dev/ttyACM0`). If you also have a Debug Probe connected
(see [07-debugging-swd.md](07-debugging-swd.md)), or more than one board,
you'll see more than one — e.g. `/dev/ttyACM0  /dev/ttyACM1`. There's no
guaranteed rule for which number goes to which device — it depends on
plug-in order, and can shift between reboots.

### Connect and check if it's the right one

Just try one:

```bash
minicom -D /dev/ttyACM0 -b 115200 -o
```

(`-o` disables minicom's modem-init string — not needed for a plain
serial device like this, and skipping it avoids some garbled-output
issues people occasionally see. This matches Raspberry Pi's own
recommended `minicom` invocation for the Debug Probe's serial port.)

If it's the target board running blink, you'll see `LED ON` / `LED OFF`
(or `bare LED ON` / `bare LED OFF` for `blink_bare`) printing every half
second. If the terminal stays completely blank, that's not necessarily
wrong — it likely means this device belongs to something else (like the
Debug Probe's own UART pass-through, which is silent unless you've
specifically wired it up). Exit (`Ctrl+A` then `X`) and try the next one:

```bash
minicom -D /dev/ttyACM1 -b 115200 -o
```

### If you want to be certain which device is which, instead of guessing

Unplug just the target board (leave anything else, like a Debug Probe,
connected), then run `ls /dev/ttyACM*` again. Whichever device
disappeared was the target's. This is slower than just trying each one,
but removes any ambiguity — useful if you have several devices connected
at once and guessing would take a while.
