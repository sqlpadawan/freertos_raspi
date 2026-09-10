# 11 — Wi-Fi Networking (Pico W / Pico 2 W only)

This is a separate, additional target — `wifi_connect` — on top of the
`blink` and `blink_bare` targets already covered elsewhere. It only
applies to **W (wireless) boards** (`pico_w`, `pico2_w`); it doesn't build
at all on plain `pico`/`pico2` boards (see "Why it doesn't build on
non-W boards" below).

## Recommended order when bringing up a new W board

If this is the first time you're running *any* of this project's code on
a particular physical board, test in this order rather than jumping
straight to `wifi_connect`:

1. **`blink_bare`** first — confirms the board, USB serial, and the
   wireless-chip LED path all work, with zero FreeRTOS involved. The
   simplest thing that could fail, so the cheapest first checkpoint.
2. **`blink`** next — adds FreeRTOS into the mix, still no networking.
3. **`wifi_connect`** last — the most complex target (FreeRTOS + USB +
   the wireless chip + the full TCP/IP stack all running together).

This isn't just caution for its own sake — it's what made the
[known RP2350 issue](10-known-issue-rp2350-freertos-usb.md) tractable to
diagnose instead of a guessing game: testing one layer at a time tells
you immediately which layer a problem is in, rather than debugging three
things at once. If something breaks with `wifi_connect`, having already
confirmed `blink_bare` and `blink` work rules out the board, USB, and
FreeRTOS itself as causes — narrowing it to Wi-Fi/lwIP specifically.

## What it does

A single FreeRTOS task joins your Wi-Fi network and prints the assigned
IP address over serial once connected — nothing more yet. It's meant as
a clean, verified starting point for adding real networking (sockets,
HTTP requests, MQTT, whatever you're building toward), not a finished
application.

## One-time setup: your Wi-Fi credentials

Credentials are **not** stored in any file this project tracks in git —
they live in a local, gitignored file you create yourself:

```bash
cd src/wifi_connect
cp wifi_config.h.example wifi_config.h
```

Edit `wifi_config.h` and fill in your actual network name and password:

```c
#define WIFI_SSID     "your-network-name"
#define WIFI_PASSWORD "your-network-password"
```

`wifi_config.h` is listed in `.gitignore` — it will never be committed,
even accidentally. `wifi_config.h.example` (the template, with placeholder
values) is what's actually tracked in git, so anyone cloning this repo
gets clear instructions without ever seeing real credentials.

**If you don't create this file, the build will fail** with a "no such
file" error on `#include "wifi_config.h"` in `src/wifi_connect/main.c` —
that's expected, and the fix is the two commands above.

## Building and flashing

Same process as the other targets, just naming this one specifically:

```bash
cmake -B build -DPICO_BOARD=pico_w -G Ninja .
ninja -C build wifi_connect
cp build/src/wifi_connect.uf2 /media/$USER/RPI-RP2/
```

Watch serial output (see [06-flashing-uf2.md](06-flashing-uf2.md)) for:

```
Connecting to Wi-Fi SSID 'your-network-name'...
Connected. IP address: 192.168.1.xxx
```

If it fails instead, you'll see `Wi-Fi connect failed, error N` — check
your SSID/password are correct, and that your network is 2.4GHz (the
CYW43439 chip on Pico W doesn't support 5GHz networks).

## Submodules: what gets fetched, and when

This target needs `lib/lwip` — the actual TCP/IP stack — on top of the
`lib/cyw43-driver` that plain LED control on W boards already needs (see
[01-hardware-setup.md](01-hardware-setup.md) and
[02-host-toolchain-setup.md](02-host-toolchain-setup.md)). Both are
fetched **automatically** the first time you configure for a `_w` board —
see the top of the repo root `CMakeLists.txt` for the logic. You'll see:

```
-- Board 'pico_w' is a wireless (W) board - fetching lib/cyw43-driver...
-- Board 'pico_w' is a wireless (W) board - fetching lib/lwip...
```

the first time only. `lwip` is a genuinely large dependency (the full
TCP/IP stack, with support for IPv6, PPP, and protocols this example
doesn't use) — there's no way to fetch a meaningfully smaller subset of
it, unlike the tinyusb-only-not-recursive approach used elsewhere in this
project's submodule strategy.

## Why it doesn't build on non-W boards

`src/CMakeLists.txt` only adds the `wifi_connect` target when pico-sdk's
own `pico_cyw43_arch_lwip_sys_freertos` CMake target exists — which only
happens on boards with `PICO_CYW43_SUPPORTED` set (the `_w` boards).
On a plain `pico`/`pico2` build, this block is silently skipped — no
error, the target just doesn't exist, matching how `blink`/`blink_bare`
already handle the LED-via-wireless-chip difference.

## The `lwipopts.h` config file, and why the exact values matter

lwIP requires a project-specific configuration header — pico-sdk doesn't
ship a default one, since the right settings depend on what you're
building. `src/wifi_connect/lwipopts.h` in this repo is **not** invented
from scratch — it's based on Raspberry Pi's own official reference
configuration for FreeRTOS + Wi-Fi examples
(`pico-examples/pico_w/wifi/lwipopts_examples_common.h` combined with
`pico-examples/pico_w/wifi/freertos/ping/lwipopts.h`), verified against
that source. Getting a setting like `NO_SYS` wrong here doesn't
necessarily fail to compile — it can cause a hang or subtle misbehavior
at runtime instead, which is a much harder class of bug to track down
(see [10-known-issue-rp2350-freertos-usb.md](10-known-issue-rp2350-freertos-usb.md)
for what that kind of debugging looks like). If you need to change these
settings, check them against
[lwIP's own options reference](https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
or a working example rather than guessing.

One deliberate scope decision worth knowing about: this config has both
`LWIP_SOCKET` and `LWIP_NETCONN` set to `0` — meaning **no BSD-socket or
netconn API is enabled**. That's fine for this example, since reading the
assigned IP address only needs `netif_ip4_addr()`, not an actual socket.
If you're adding real networking (opening a TCP connection, an HTTP
request, etc.), you'll need to set `LWIP_SOCKET` to `1` — see the comment
in `lwipopts.h` for the one other setting (`MEM_LIBC_MALLOC`) that
interacts with this choice.

## Extending this into real networking

This target is a foundation, not a finished feature. Natural next steps,
roughly in order of complexity:

1. **Sockets** — set `LWIP_SOCKET` to `1` in `lwipopts.h`, then use
   standard BSD socket calls (`socket()`, `connect()`, `send()`, `recv()`)
   from `wifi_connect_task` or a new task.
2. **HTTP client** — pico-sdk includes lwIP's `httpc` module; see
   `pico-examples/pico_w/wifi/http_client` upstream for a reference
   (not included in this repo).
3. **MQTT, NTP, or other protocols** — lwIP includes modules for several
   of these; each needs its own `lwipopts.h` additions, following the
   same pattern as the settings already here.

If you pursue any of these, treat it the same way this target was built:
verify a real, minimal, working version before layering on more, and
check specific settings against a real reference rather than guessing —
lwIP's option surface is large, and subtle misconfigurations tend to fail
as runtime hangs rather than build errors.
