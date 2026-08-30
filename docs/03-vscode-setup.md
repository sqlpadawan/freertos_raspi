# 03 — VS Code Setup

> VS Code itself is installed automatically by
> [`scripts/setup-host.sh`](../scripts/setup-host.sh) if it isn't already
> present — see [02-host-toolchain-setup.md](02-host-toolchain-setup.md#installing-vs-code).
> This doc is about configuring it once it's installed.

This is a strict, ordered checklist. Do these once, in this exact order,
before you try to build anything from inside VS Code. Skipping a step or
doing them out of order is the most common source of confusing errors here.

## First-time setup (do this once)

1. **Open the correct folder.**
   From a terminal, `cd` into the repo root (the folder containing
   `CMakeLists.txt`, `docs/`, `src/`, etc.) and run:
   ```bash
   code .
   ```
   Or in VS Code: `File → Open Folder…` → select that same folder. It must
   be the repo root — not `src/`, not a parent folder containing it.

2. **Install the recommended extensions.**
   A popup should appear in the bottom-right: *"This workspace has
   extension recommendations."* Click **Install All**.
   If the popup doesn't appear (or you dismissed it), open the Extensions
   panel (`Ctrl+Shift+X`), type `@recommended` in the search box, and
   install the four listed there. See the [table below](#extensions) for
   what each one does if you'd rather install manually via the CLI.

3. **Select the correct CMake kit.**
   Open the Command Palette (`Ctrl+Shift+P`), type and run:
   ```
   CMake: Select a Kit
   ```
   A list of detected compilers appears. Pick the one that mentions
   **`arm-none-eabi`** (e.g. *"GCC 13.2.1 arm-none-eabi"*) — **not** your
   Pi's native `gcc`/`g++`. If no `arm-none-eabi` entry shows up at all:
   - Confirm the toolchain is actually installed: `arm-none-eabi-gcc --version`
     in a terminal. If that fails, re-run `scripts/setup-host.sh`.
   - Run `CMake: Scan for Kits` from the Command Palette first, then repeat
     `CMake: Select a Kit`.

4. **Configure the project.**
   Command Palette → run:
   ```
   CMake: Configure
   ```
   Watch the **Output** panel (dropdown at top of the panel should say
   "CMake/Build") for the same "Configuring done" / "Generating done"
   messages you'd see running `cmake` by hand (see
   [05-building.md](05-building.md)). If it errors, the message there will
   tell you what's wrong — usually a missing kit (step 3) or a submodule
   that hasn't been fetched yet — see
   [02-host-toolchain-setup.md](02-host-toolchain-setup.md#sdks-pulled-in-as-submodules)).

That's it for one-time setup. Steps 3–4 only need to be redone if you
change boards (`PICO_BOARD`) or delete the `build/` folder.

## Everyday workflow (after first-time setup)

| Action | How |
|---|---|
| **Build** | `Ctrl+Shift+B` — runs this repo's "Build" task (`cmake --build build`) |
| **Flash via SWD** | Command Palette → `Tasks: Run Task` → **"Flash (OpenOCD, SWD)"** |
| **Debug** | Open the "Run and Debug" panel (`Ctrl+Shift+D`), pick a launch config from the dropdown at the top ("Debug (Debug Probe / SWD)" or "Debug (Direct Pi GPIO / SWD)"), press the green ▶ |
| **Flash via UF2 (no probe)** | Not a VS Code action — see [06-flashing-uf2.md](06-flashing-uf2.md), it's just copying a file |

You do **not** need to use the CMake Tools status-bar buttons (the ones
at the bottom of the window labeled with a kit name / build variant /
target). This repo's own Build task and the CMake Tools "Build" button
both end up running the same underlying command against the same
`build/` folder, so either works — but the keyboard shortcut and the
instructions in this repo's docs all assume you're using `Ctrl+Shift+B`
and the Command Palette, not those buttons, to keep things consistent.

## Extensions

| Extension | ID | Purpose |
|---|---|---|
| Raspberry Pi Pico | `raspberry-pi.raspberry-pi-pico` | Official extension: project creation, build, flash, debug integration |
| C/C++ | `ms-vscode.cpptools` | IntelliSense, debugging |
| CMake Tools | `ms-vscode.cmake-tools` | CMake integration (kit selection, configure) |
| Cortex-Debug | `marus25.cortex-debug` | SWD/GDB debugging UI (register/peripheral views) |

Install manually via CLI if needed:

```bash
code --install-extension raspberry-pi.raspberry-pi-pico
code --install-extension ms-vscode.cpptools
code --install-extension ms-vscode.cmake-tools
code --install-extension marus25.cortex-debug
```

## Why the official "Raspberry Pi Pico" extension, if we don't use its buttons

It bundles a known-good pico-sdk/toolchain version and can manage the whole
build for you, but this repo intentionally keeps its **own** CMake/SDK
setup independent of the extension's managed environment, so the project
still builds correctly from the command line or CI, not just inside VS
Code. Having the extension installed is still useful — it supplies bundled
OpenOCD config fragments and Pico-specific tooling other parts of this
setup rely on — you just don't need to click through its own project/build
UI for this repo specifically.

## What's in `.vscode/`, for reference

- **`settings.json`** — points CMake Tools at the Ninja generator and a
  fixed `build/` directory, and tells C/C++ IntelliSense where to find
  the Pico SDK and FreeRTOS headers so autocomplete works across both.
- **`tasks.json`** — defines the "Build" task (bound to `Ctrl+Shift+B`)
  and the "Flash (OpenOCD, SWD)" task described above.
- **`launch.json`** — the two Cortex-Debug configurations for the two SWD
  wiring options, see [07-debugging-swd.md](07-debugging-swd.md).
- **`extensions.json`** — the recommendation list from step 2 above.

If something here doesn't behave as described, check
[09-troubleshooting.md](09-troubleshooting.md) before assuming it's a typo
in your setup — a few sharp edges in this exact flow are already documented
there.
