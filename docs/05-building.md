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
