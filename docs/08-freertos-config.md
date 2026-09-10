# 08 — FreeRTOS Configuration

> **Known issue:** the settings below let FreeRTOS build and run correctly
> on Pico 2 (RP2350), but there's a separate, unresolved issue where
> FreeRTOS + USB serial hangs on that chip specifically — not caused by
> anything in this file. See
> [10-known-issue-rp2350-freertos-usb.md](10-known-issue-rp2350-freertos-usb.md).

`FreeRTOSConfig.h` at the repo root controls kernel behavior. Key settings
used in this project, and why:

| Setting | Value | Reason |
|---|---|---|
| `configUSE_PREEMPTION` | `1` | Standard preemptive scheduling |
| `configCPU_CLOCK_HZ` | `133000000` (Pico) / `150000000` (Pico 2) | Must match `SystemCoreClock`; update if you overclock |
| `configTICK_RATE_HZ` | `1000` | 1ms tick — fine enough for most demos without excessive overhead |
| `configMAX_PRIORITIES` | `5` | Plenty for small demos; raise if you need finer priority separation |
| `configMINIMAL_STACK_SIZE` | `128` (words) | Stack for the idle task and a sane minimum for small tasks. 128 words = 512 bytes on this 32-bit target — fine for blink, but bump it up for tasks doing real work (e.g. printf-heavy code, floating point) |
| `configTOTAL_HEAP_SIZE` | `(128*1024)` | FreeRTOS heap (heap_4). RP2040 has 264KB SRAM total — leave headroom |
| `configUSE_TIMERS` | `1` | Needed if you use software timers |
| `INCLUDE_xTimerPendFunctionCall` | `1` | Required by the RP2040 FreeRTOS port itself (`port.c` calls `xTimerPendFunctionCallFromISR` internally for inter-core spinlock notification) — leaving this at the default `0` causes a link error even if your own code never touches timers |
| `INCLUDE_xSemaphoreGetMutexHolder` | `1` | Required by pico-sdk's `pico_async_context_freertos` (used by the `wifi_connect` target on W boards, see [11-wifi-networking.md](11-wifi-networking.md)) — causes a link error (`undefined reference to xSemaphoreGetMutexHolder`) without it, even though `blink`/`blink_bare` don't need this specific setting themselves |
| `configSUPPORT_DYNAMIC_ALLOCATION` | `1` | Required for `xTaskCreate` (vs. only `xTaskCreateStatic`) |
| `configNUM_CORES` | `1` | Single-core scheduling. Set to `2` to use SMP FreeRTOS across both RP2040/RP2350 cores (bigger change — see below) |
| `configENABLE_FPU` | `1` (Pico 2 / RP2350 only) | The Cortex-M33 port headers `#error` out if this isn't defined at all, even on RP2040 builds where it's simply unused. RP2350 has a hardware FPU; enabling it lets FreeRTOS save/restore FPU context correctly on task switches |
| `configENABLE_MPU` | `0` (Pico 2 / RP2350 only) | Memory Protection Unit support — off by default; enabling it is a bigger step involving per-task memory regions, not needed for this demo |
| `configENABLE_TRUSTZONE` | `0` (Pico 2 / RP2350 only) | Non-secure-only build — TrustZone secure/non-secure world splitting isn't used here |
| `configMAX_SYSCALL_INTERRUPT_PRIORITY` | `(5 << 5)` (Pico 2 / RP2350 only) | Required by the Cortex-M33 port's critical-section implementation. Not used on the original Pico's Cortex-M0+, which has no BASEPRI register |

## Single-core vs SMP (dual-core)

The RP2040/RP2350 port of FreeRTOS supports SMP (both cores running the
scheduler). This repo defaults to **single-core** for simplicity and
easier debugging. To move to SMP later:

1. Set `configNUM_CORES` to `2` in `FreeRTOSConfig.h`
2. Set `configTICK_CORE` and `configRUN_MULTIPLE_PRIORITIES` as needed
3. Pin core-affinity-sensitive tasks with `vTaskCoreAffinitySet`

This is a meaningful architecture decision — don't flip it on without
reading the port's SMP notes in `lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/`.

## Heap allocator

This project uses `heap_4.c` (from `lib/FreeRTOS-Kernel/portable/MemMang/`),
which supports coalescing freed blocks — a reasonable default unless you
have a specific reason to use `heap_1` (no free) or `heap_2` (no coalescing).

## Where the RTOS port glue lives

`lib/FreeRTOS-Kernel/portable/ThirdParty/GCC/RP2040/` — this is what wires
FreeRTOS's `portable.h` layer to the RP2040/RP2350 SDK (interrupt vectors,
context switching, SysTick). You generally shouldn't need to touch this.

## W (wireless) boards

Pico W and Pico 2 W wire the onboard LED through the CYW43439
wireless/Bluetooth chip instead of a plain GPIO pin. `src/main.c` and
`src/blink_bare/main.c` both handle this with a compile-time check —
the same pattern used by pico-sdk's own official examples:

```c
#if defined(CYW43_WL_GPIO_LED_PIN)
    // W board: LED is behind the wireless chip driver
    cyw43_arch_init();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#elif defined(PICO_DEFAULT_LED_PIN)
    // Plain board: LED is a normal GPIO
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#endif
```

`CYW43_WL_GPIO_LED_PIN` is only defined by pico-sdk's board header when
you build with `PICO_BOARD=pico_w` or `pico2_w` — so the same source file
correctly targets either kind of board depending on which one you
configure for, with no manual code changes needed.

`src/CMakeLists.txt` conditionally links `pico_cyw43_arch_none` — the
minimal variant of pico-sdk's wireless-chip library that only provides
GPIO control, without pulling in the full Wi-Fi/Bluetooth networking
stack (which needs `lwip`, a much larger dependency this project doesn't
otherwise fetch — see [02-host-toolchain-setup.md](02-host-toolchain-setup.md)).
The `if (TARGET pico_cyw43_arch_none)` guard means this only takes effect
when building for a W board; it's silently skipped otherwise.

**One extra submodule is needed for W boards:** `lib/cyw43-driver`,
nested inside `pico-sdk` (similar to `lib/tinyusb`, which this project
always fetches). Fetch it once:

```bash
git -C lib/pico-sdk submodule update --init lib/cyw43-driver
```

`setup-host.sh` does **not** fetch this by default, since it's only
needed for W boards specifically — the same reasoning as skipping
`lib/lwip`, `lib/mbedtls`, and `lib/btstack`.
