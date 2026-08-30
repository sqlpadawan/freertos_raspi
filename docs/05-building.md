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

**If you only ever build for one board, skip straight to "From the command
line" below and don't worry about separate build directories.** They start
to matter once you're going back and forth between a Pico and a Pico 2.

### Why board switches need a clean build directory

RP2040 (Pico) and RP2350 (Pico 2) are different chip platforms
(`rp2040` vs. `rp2350`), not just a board-name flag. If you reconfigure an
*existing* `build/` folder that was already set up for one platform and
point it at the other, pico-sdk itself refuses:

```
CMake Error: PICO_PLATFORM is specified to be 'rp2040', but PICO_BOARD='pico2'
uses 'rp2350' which is incompatible. You need to delete the CMake cache or
build directory and reconfigure to proceed. The best practice is to use
separate build directories for different platforms.
```

That message is pico-sdk's own guidance, not a bug — the fix depends on
whether you switch boards occasionally or work with both regularly:

**Switching occasionally (delete and reconfigure):**

```bash
rm -rf build
cmake -B build -DPICO_BOARD=pico2 -G Ninja .
ninja -C build
```

> ⚠️ **WARNING — irreversible:** `rm -rf build` permanently deletes the
> build directory. Safe here since it's generated/git-ignored — see the
> full warning in "Clean rebuild" below before running this if you're
> unsure.

**Working with both boards regularly (separate build directories,
pico-sdk's recommended approach):**

```bash
cmake -B build-pico   -DPICO_BOARD=pico  -G Ninja .
cmake -B build-pico2  -DPICO_BOARD=pico2 -G Ninja .

ninja -C build-pico    # build for the original Pico
ninja -C build-pico2   # build for the Pico 2
```

Both stay configured simultaneously — no deleting anything to switch
between them, just point `ninja -C` (or your flashing/debug commands) at
whichever directory matches the board you're working with. If you go this
route, update `.vscode/tasks.json` and `.vscode/launch.json` (which
currently assume a single `build/` directory) to match, or just keep using
the single-`build/`-directory workflow from VS Code and reserve the
two-directory approach for the command line.

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

**If `build/` was previously configured for the other board, see "Why
board switches need a clean build directory" above** — a plain re-run
without deleting the cache will fail with a `PICO_PLATFORM` mismatch
error, not silently do the wrong thing.

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

### Pico 2 (RP2350) only: `picotool` gets built from source on first configure

You'll see a warning like this during `cmake` configure, not `ninja` build:

```
CMake Warning at lib/pico-sdk/tools/Findpicotool.cmake:30 (message):
  No installed picotool with version 2.3.0 found - building from source
```

followed later by `Downloading Picotool`. This is expected and normal —
`picotool` is a helper tool pico-sdk needs for the Pico 2's boot stage 2
setup, and it doesn't ship pre-built for your system, so CMake fetches and
builds it as a dependency during configure. This only happens on RP2350
targets (Pico 2); it doesn't happen when building for the original Pico.

This adds noticeable time to your **first** `cmake -B build ...` configure
step for a Pico 2 build (the download + build of `picotool` itself,
separate from your own project's build time above) — it's not stuck, just
working through an extra step. Subsequent configures in the same `build/`
directory reuse it and won't repeat this.

If you build for Pico 2 often and want to skip this step, you can install
`picotool` once yourself (check `apt-cache search picotool` on Raspberry
Pi OS) or build it once separately and point pico-sdk at it via
`PICOTOOL_FETCH_FROM_GIT_PATH`, per the warning message's own suggestion —
neither is necessary, just a time-saver.
