# 05 — Building

## From the command line

```bash
mkdir -p build
cd build
cmake -G Ninja ..
ninja
```

Output artifacts land in `build/src/`:

- `blink.elf` — used for debugging (has symbols)
- `blink.uf2` — used for drag-and-drop flashing
- `blink.bin` / `blink.hex` — raw images, occasionally useful for other tools

## From VS Code

`Ctrl+Shift+B` runs the default build task defined in
[`.vscode/tasks.json`](../.vscode/tasks.json), which wraps the same
`cmake --build build` command.

## Switching the target board (Pico ↔ Pico 2)

### From the command line

Pass `-DPICO_BOARD` when configuring:

```bash
cmake -B build -DPICO_BOARD=pico2 -G Ninja .
```

Run this from the **repo root** (not from inside `build/`) — note the
trailing `.`, not `..`, since `-B build` already tells CMake where the
build directory is, and `.` here means "the source is the current
directory." (This differs from the `cd build && cmake -G Ninja ..` form
shown above, where `..` makes sense because you've already `cd`ed into
`build/` first — the two forms aren't interchangeable by just swapping
`.`/`..`.)

Use `pico` for the original Pico (RP2040, the default if you don't pass
this flag at all) or `pico2` for the Pico 2 (RP2350).

**Do a clean rebuild after switching** — see the "Clean rebuild" section
below. CMake caches `PICO_BOARD` from the first configure, so re-running
`cmake` in the same `build/` folder without clearing it first will
silently keep using whichever board you configured originally, even if
you pass a different `-DPICO_BOARD` value the second time.

### From VS Code

The board is set in two places that need to agree:

1. **`.vscode/settings.json`** — `cmake.configureArgs` hardcodes
   `-DPICO_BOARD=pico`. Change it to `-DPICO_BOARD=pico2` if you're on a
   Pico 2.
2. Then delete the cache and reconfigure so the new value actually takes
   effect: Command Palette → `CMake: Delete Cache and Reconfigure`.
   (Just running `CMake: Configure` again isn't enough — same caching
   behavior as the command-line case above.)

## Checking which board you're currently configured for

Either of these tells you what's actually configured right now, without
needing to remember what you last typed:

```bash
grep PICO_BOARD build/CMakeCache.txt
```

or look for this line in the configure output (VS Code: Output panel,
"CMake/Build" channel; command line: printed directly):

```
Target board (PICO_BOARD) is 'pico2'.
```

If `build/` doesn't exist yet or was deleted, there's nothing configured
yet — run `CMake: Configure` (or `cmake -B build -G Ninja ..` on the
command line) first.

## Clean rebuild

If you change `PICO_BOARD`, submodule versions, or the toolchain, do a
clean rebuild rather than an incremental one, since stale CMake cache
entries are a common source of confusing errors:

```bash
rm -rf build
```

> ⚠️ **WARNING — irreversible:** `rm -rf build` permanently deletes the
> entire build output directory. This is safe in this project because
> `build/` contains only generated artifacts (it's git-ignored, nothing
> here is source-controlled or otherwise unrecoverable) — but always
> confirm you're deleting the intended folder and not, e.g., running this
> from the wrong working directory. Dry-run first if unsure:
> ```bash
> ls build   # confirm this is really the generated build folder
> ```
> then proceed with the `rm -rf build` above.

## Build times

First build compiles all of `pico-sdk` and `FreeRTOS-Kernel` from scratch
and can take several minutes on a Pi 5. Incremental builds after that are
fast (a few seconds).
