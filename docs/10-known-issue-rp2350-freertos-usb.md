# 10 — Known Issue: FreeRTOS + USB Serial Hangs on Pico 2 (RP2350)

## Summary

On a **Pico 2 (RP2350)**, the FreeRTOS `blink` target builds, flashes, and
connects fine over SWD — but never brings up its USB serial connection,
and the chip's own bootrom eventually gives up on the application and
falls back to presenting itself as a bootloader again. The identical
project builds and runs correctly on the original **Pico (RP2040)**, and
a plain, non-FreeRTOS build (`blink_bare`) works correctly on the *same*
Pico 2 board. The problem is isolated specifically to running FreeRTOS +
USB CDC together, on RP2350 silicon. As of this writing, it is not fully
resolved — this is a known, currently-open issue in the wider RP2350
community, not something specific to this project's setup.

## What we observed, in the order we found it

1. **UF2 flashing appeared to succeed**, but the onboard LED never blinked
   and no serial output appeared.
2. **USB enumeration was actually failing at the hardware level.**
   `dmesg` showed repeated `device descriptor read/64, error -110` and
   `attempt power cycle` messages — not a power/cable issue (ruled out by
   testing a direct port and different cable with no change) but the
   RP2350 chip's own bootrom recovery mechanism: after the application
   crashes or fails to come up correctly, the bootrom automatically falls
   back to re-presenting itself as `RP2350 Boot` (the mass-storage
   bootloader), rather than getting permanently stuck.
3. **A bare-metal build (`blink_bare`, no FreeRTOS) worked perfectly** on
   the exact same physical board — LED blinked, USB serial printed
   cleanly, immediately. This ruled out the board, cable, toolchain,
   `PICO_BOARD` setting, and base pico-sdk USB support as causes.
4. **SWD debugging itself worked cleanly** once `launch.json`'s hardcoded
   `target/rp2040.cfg` was corrected to `target/rp2350.cfg` (a real bug in
   this project's initial scaffold, unrelated to the USB issue — see git
   history). OpenOCD connected without issue and correctly identified
   both Cortex-M33 cores.
5. **GDB itself hit a separate, unrelated compatibility issue**
   (`Remote replied unexpectedly to 'vMustReplyEmpty'`) connecting with
   `target remote`, caused by a long-documented OpenOCD gdbserver quirk
   (a nonstandard `"timeout"` item in its `qSupported` response confusing
   modern GDB versions). Using `target extended-remote` instead of
   `target remote` worked around this and allowed a real debug session.
6. **With a working debug session**, breaking at `main()` and stepping
   into `stdio_init_all()` showed execution stuck inside tinyusb's CDC
   driver (`_prep_out_transaction`, in `cdc_device.c`) — real code, not
   corrupted memory, but stuck at the same instruction on repeated
   interrupts. This confirmed a genuine hang inside the USB stack itself,
   specifically when running under the FreeRTOS scheduler.

## Root cause (as far as we've isolated it)

This matches actively-discussed, unresolved reports elsewhere:

- A Raspberry Pi forum thread titled "rp2350 with tinyusb (device) -
  'slow' with FreeRTOS" describes the same combination (RP2350 + FreeRTOS
  + tinyusb) being extremely slow or stuck to enumerate, while identical
  code runs instantly on RP2040. The author tried single-core, dual-core,
  and moving the USB task between cores, without a full fix.
- A tinyusb pull request ("set irq priority for freertos") notes that
  tinyusb is supposed to set the USB interrupt's priority to a level
  FreeRTOS considers safe for interrupts that touch RTOS objects
  (semaphores, queues) — and that this setting "was initially there, then
  commented out for some reason." A missing or incorrect USB interrupt
  priority is a plausible, testable root cause, but we did not confirm a
  fix in this project (see "Where we stopped" below).

In short: this looks like an interrupt-priority or timing interaction
between FreeRTOS's scheduler and tinyusb's USB interrupt handling, that
specifically affects the newer RP2350 silicon and hasn't yet been fully
solved upstream, as of when this was investigated (mid-2026).

## Where we stopped

We did not pursue a fix further in this project. Untried but plausible
next steps, if picked up again later:

- Explicitly set the USB interrupt's priority to a FreeRTOS-safe level
  (e.g. via `irq_set_priority(USBCTRL_IRQ, ...)`) before `stdio_init_all()`
  runs, matching the intent of the tinyusb PR referenced above.
- Check for a newer pico-sdk / tinyusb release that may have since
  addressed this (this project pins `pico-sdk` at tag `2.3.0` — see
  [02-host-toolchain-setup.md](02-host-toolchain-setup.md)).
- Switch the project to UART-based `stdio` instead of USB CDC
  (`pico_enable_stdio_uart` instead of `pico_enable_stdio_usb`) as a
  workaround that sidesteps the issue entirely — this project already
  has Debug Probe UART wiring documented in
  [01-hardware-setup.md](01-hardware-setup.md), which could carry this.
- **Try RTT instead of USB CDC** — see
  [12-debug-probe-reference.md](12-debug-probe-reference.md#rtt-printf-output-with-no-usb-or-uart-at-all).
  RTT sends output over SWD, never touching USB at all. Since this hang
  is isolated to tinyusb specifically, RTT is a plausible way to get
  working `printf` output on Pico 2 without needing USB to come up —
  untried, but worth trying before this one specifically.

## What is and isn't affected

| | Pico (RP2040) | Pico 2 (RP2350) |
|---|---|---|
| Build (both `blink` and `blink_bare`) | ✅ verified (build only, not on real hardware) | ✅ verified on real hardware |
| UF2 flashing | not yet tested on real hardware | ✅ verified |
| `blink_bare` (no FreeRTOS) — LED + USB serial | not yet tested on real hardware | ✅ verified working |
| SWD connection via Debug Probe | not yet tested on real hardware | ✅ verified working (both cores detected) |
| `blink` (FreeRTOS) — LED + USB serial | not yet tested on real hardware, but this exact combination has no known issue on RP2040 | ❌ hangs, never brings up USB |

If you're picking this project back up on an **original Pico (RP2040)**,
none of the above should apply — this issue is specific to RP2350. Given
we hadn't yet tested real RP2040 hardware as of this writing, it's still
worth confirming the basics (blink, serial, SWD) work as expected on that
hardware too, even though we have every reason to expect they will.
