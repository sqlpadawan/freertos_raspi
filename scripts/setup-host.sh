#!/usr/bin/env bash
#
# Bootstraps a fresh Raspberry Pi OS install for FreeRTOS-on-Pico development.
# Safe to re-run: every step checks whether it's already done before acting.
#
# See docs/02-host-toolchain-setup.md for what this does and why.

set -euo pipefail

if [ "$(id -u)" -eq 0 ]; then
    echo "ERROR: Don't run this script with sudo / as root." >&2
    echo "It escalates internally (via sudo) only for the specific steps" >&2
    echo "that need it (apt, usermod, make install). Running the whole" >&2
    echo "script as root causes problems later — e.g. VS Code refuses to" >&2
    echo "run as root and prints 'trying to start Visual Studio Code as a" >&2
    echo "super user' warnings when the script tries to install extensions." >&2
    echo "" >&2
    echo "Re-run as your normal user instead:" >&2
    echo "    ./scripts/setup-host.sh" >&2
    exit 1
fi

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "==> [1/7] Installing apt packages"
sudo apt-get update
sudo apt-get install -y \
    cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi \
    build-essential git python3 gdb-multiarch minicom \
    libusb-1.0-0-dev libhidapi-dev libgpiod-dev pkg-config autoconf \
    libtool texinfo

echo "==> [2/7] Fetching git submodules (pico-sdk, FreeRTOS-Kernel)"
# NOTE: intentionally NOT using --recursive here. pico-sdk has several of
# its own nested submodules (cyw43-driver, lwip, mbedtls, btstack) that are
# only needed for Pico W wireless/Bluetooth features and are large. We only
# need pico-sdk's tinyusb submodule for USB stdio (printf), so fetch that
# one specifically instead. See docs/02-host-toolchain-setup.md.
git submodule update --init
git -C lib/pico-sdk submodule update --init lib/tinyusb

echo "==> [3/7] Building Raspberry Pi fork of OpenOCD (with linuxgpiod support)"
if command -v openocd >/dev/null 2>&1 && openocd --version 2>&1 | grep -qi "rp2040\|rp2350\|raspberrypi"; then
    echo "    OpenOCD with RP2040/RP2350 support already installed, skipping build."
else
    # NOTE: Raspberry Pi's fork retired the old "rp2040" branch. The current
    # default branch is "rpi-common" and supports both RP2040 and RP2350, so
    # no --branch flag is needed. See docs/09-troubleshooting.md if this
    # ever changes again upstream.
    if [ ! -d "$HOME/openocd-rpi-src" ]; then
        git clone https://github.com/raspberrypi/openocd.git "$HOME/openocd-rpi-src"
    fi
    (
        cd "$HOME/openocd-rpi-src"
        git submodule update --init          # pulls in the jimtcl submodule
        ./bootstrap
        ./configure --enable-linuxgpiod --enable-cmsis-dap --enable-internal-jimtcl
        make -j"$(nproc)"
        sudo make install
    )
fi

echo "==> [4/7] Setting environment variables in ~/.bashrc"
BASHRC="$HOME/.bashrc"
add_export_once() {
    local line="$1"
    if ! grep -qxF "$line" "$BASHRC" 2>/dev/null; then
        echo "$line" >> "$BASHRC"
        echo "    Added: $line"
    else
        echo "    Already present: $line"
    fi
}
add_export_once "export PICO_SDK_PATH=\"$REPO_ROOT/lib/pico-sdk\""
add_export_once "export FREERTOS_KERNEL_PATH=\"$REPO_ROOT/lib/FreeRTOS-Kernel\""

echo "==> [5/7] Adding user to dialout/gpio groups (for serial + GPIO access)"
sudo usermod -a -G dialout,gpio "$USER" || true

echo "==> [6/7] Installing VS Code (if not already installed)"
if command -v code >/dev/null 2>&1; then
    echo "    VS Code already installed, skipping."
else
    # Raspberry Pi OS (Bookworm and later) ships VS Code directly in its own
    # apt repos for arm64/armhf, so no need to add Microsoft's repo manually.
    # See: https://www.raspberrypi.com/news/coding-on-raspberry-pi-with-visual-studio-code/
    if sudo apt-get install -y code; then
        echo "    Installed VS Code from Raspberry Pi OS apt repo."
    else
        echo "    'code' not found in apt repos on this OS/arch."
        echo "    Falling back to Microsoft's apt repo..."
        sudo apt-get install -y wget gpg apt-transport-https
        wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > /tmp/packages.microsoft.gpg
        sudo install -D -o root -g root -m 644 /tmp/packages.microsoft.gpg /etc/apt/keyrings/packages.microsoft.gpg
        echo "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" | \
            sudo tee /etc/apt/sources.list.d/vscode.list > /dev/null
        sudo apt-get update
        sudo apt-get install -y code
    fi
fi

echo "==> [7/7] Installing VS Code extensions"
if command -v code >/dev/null 2>&1; then
    code --install-extension raspberry-pi.raspberry-pi-pico || true
    code --install-extension ms-vscode.cpptools || true
    code --install-extension ms-vscode.cmake-tools || true
    code --install-extension marus25.cortex-debug || true
else
    echo "    'code' CLI still not found — install VS Code manually, see docs/03-vscode-setup.md"
fi

cat <<'EOF'

==> Setup complete.

Next steps:
  1. Close and reopen your terminal (or: source ~/.bashrc)
  2. Log out and back in for the dialout/gpio group changes to apply
  3. Verify the toolchain: see docs/02-host-toolchain-setup.md "Verifying the install"
  4. Open the project: code .
  5. Build: Ctrl+Shift+B, then flash via docs/06-flashing-uf2.md
EOF
