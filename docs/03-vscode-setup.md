# 03 — VS Code Setup

## Extensions

Installed automatically via `.vscode/extensions.json` recommendations
(VS Code will prompt to install them on first open), or manually:

| Extension | ID | Purpose |
|---|---|---|
| Raspberry Pi Pico | `raspberry-pi.raspberry-pi-pico` | Official extension: project creation, build, flash, debug integration |
| C/C++ | `ms-vscode.cpptools` | IntelliSense, debugging |
| CMake Tools | `ms-vscode.cmake-tools` | CMake integration |
| Cortex-Debug | `marus25.cortex-debug` | SWD/GDB debugging UI (register/peripheral views) |

Install manually with:

```bash
code --install-extension raspberry-pi.raspberry-pi-pico
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension marus25.cortex-debug
```

## Why the official "Raspberry Pi Pico" extension

It bundles a known-good pico-sdk/toolchain version and handles the CMake
configure step, but this repo intentionally keeps its **own** CMake/SDK
setup independent of the extension's managed environment, so the project
still builds correctly from the command line or CI, not just inside VS Code.
The extension is used here mainly for its build/flash/debug UI buttons and
its bundled OpenOCD config fragments.

## Workspace settings (`.vscode/settings.json`)

Points CMake Tools at the correct toolchain file and kit, and tells
C/C++ IntelliSense where to find the Pico SDK and FreeRTOS headers so
autocomplete and go-to-definition work across both.

## Tasks (`.vscode/tasks.json`)

Two tasks are defined:

- **Build** — runs `cmake --build build`
- **Flash (OpenOCD)** — runs the OpenOCD flash command for whichever debug
  method you set up in [07-debugging-swd.md](07-debugging-swd.md)

Run via `Terminal → Run Task...` or `Ctrl+Shift+B` for the default build task.

## Debug config (`.vscode/launch.json`)

A Cortex-Debug configuration pointed at `gdb-multiarch` and OpenOCD,
targeting either RP2040 or RP2350 depending on which board you're using
(see the `device` field — update it to match your board).

## First-time checklist

- [ ] Open the folder (`code .`) at the repo root, not a subfolder
- [ ] Accept the extension recommendations prompt
- [ ] `Ctrl+Shift+P` → `CMake: Select a Kit` → choose the ARM GCC kit
- [ ] `Ctrl+Shift+P` → `CMake: Configure`
- [ ] `Ctrl+Shift+B` to build
