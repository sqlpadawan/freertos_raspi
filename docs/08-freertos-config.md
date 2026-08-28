# 08 — FreeRTOS Configuration

`FreeRTOSConfig.h` at the repo root controls kernel behavior. Key settings
used in this project, and why:

| Setting | Value | Reason |
|---|---|---|
| `configUSE_PREEMPTION` | `1` | Standard preemptive scheduling |
| `configCPU_CLOCK_HZ` | `133000000` (Pico) / `150000000` (Pico 2) | Must match `SystemCoreClock`; update if you overclock |
| `configTICK_RATE_HZ` | `1000` | 1ms tick — fine enough for most demos without excessive overhead |
| `configMAX_PRIORITIES` | `5` | Plenty for small demos; raise if you need finer priority separation |
| `configTOTAL_HEAP_SIZE` | `(128*1024)` | FreeRTOS heap (heap_4). RP2040 has 264KB SRAM total — leave headroom |
| `configUSE_TIMERS` | `1` | Needed if you use software timers |
| `configSUPPORT_DYNAMIC_ALLOCATION` | `1` | Required for `xTaskCreate` (vs. only `xTaskCreateStatic`) |
| `configNUM_CORES` | `1` | Single-core scheduling. Set to `2` to use SMP FreeRTOS across both RP2040/RP2350 cores (bigger change — see below) |

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
