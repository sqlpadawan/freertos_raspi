# 04 — Project Structure

```
.
├── CMakeLists.txt              # Top-level build config, board selection
├── pico_sdk_import.cmake       # Standard shim that locates $PICO_SDK_PATH
├── FreeRTOSConfig.h            # RTOS kernel config (tick rate, heap size, etc.)
├── src/
│   ├── CMakeLists.txt          # App-level target definitions
│   ├── main.c                  # Blink example: one FreeRTOS task toggles the LED
│   ├── blink_bare/
│   │   └── main.c              # Diagnostic target: same blink, no FreeRTOS (see docs/09-troubleshooting.md)
│   └── wifi_connect/           # W boards only — see docs/11-wifi-networking.md
│       ├── main.c
│       ├── lwipopts.h          # Required lwIP config, based on official reference
│       ├── wifi_config.h.example  # Copy to wifi_config.h and fill in your credentials
│       └── wifi_config.h       # Not tracked in git — your actual credentials
├── lib/                        # Git submodules (added on first setup)
│   ├── pico-sdk/
│   └── FreeRTOS-Kernel/
├── .vscode/
│   ├── settings.json
│   ├── tasks.json
│   ├── launch.json
│   └── extensions.json
├── scripts/
│   └── setup-host.sh
└── docs/                       # You are here
```

## Selecting the board

`CMakeLists.txt` sets `PICO_BOARD` based on a cache variable. To target
Pico 2 instead of the original Pico:

```bash
cmake -B build -DPICO_BOARD=pico2 -G Ninja .
```

Default is `pico` (original RP2040 board) if unspecified.

## Adding a new example

This repo currently has two targets: `blink` (the main FreeRTOS example)
and `blink_bare` (a bare-metal, no-FreeRTOS diagnostic — see
[09-troubleshooting.md](09-troubleshooting.md)). To add another FreeRTOS
example (e.g. a queue/semaphore demo):

1. Create `src/queue_demo/main.c`
2. Add a matching `add_executable(...)` block in `src/CMakeLists.txt`
3. Re-run `CMake: Configure` in VS Code, or `cmake -B build`

Keep each example as its own executable target rather than branching
behavior with `#ifdef`s — it keeps `main.c` files readable and each one
independently flashable.
