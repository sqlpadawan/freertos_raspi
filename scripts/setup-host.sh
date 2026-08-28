#!/usr/bin/env bash
#
# Bootstraps a fresh Raspberry Pi OS install for FreeRTOS-on-Pico development.
# Safe to re-run: every step checks whether it's already done before acting.
#
# See docs/02-host-toolchain-setup.md for what this does and why.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "==> [1/6] Installing apt packages"
sudo apt-get update
sudo apt-get install -y \
    cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi \
    build-essential git python3 gdb-multiarch minicom \
    libusb-1.0-0-dev libhidapi-dev libgpiod-dev pkg-config autoconf \
    libtool texinfo

echo "==> [2/6] Fetching git submodules (pico-sdk, FreeRTOS-Kernel)"
git submodule update --init --recursive

echo "==> [3/6] Building Raspberry Pi fork of OpenOCD (with linuxgpiod support)"
if command -v openocd >/dev/null 2>&1 && openocd --version 2>&1 | grep -qi "rp2040\|raspberrypi"; then
    echo "    OpenOCD with RP2040 support already installed, skipping build."
else
    if [ ! -d "$HOME/openocd-rpi-src" ]; then
        git clone https://github.com/raspberrypi/openocd.git "$HOME/openocd-rpi-src" \
            --branch rp2040 --depth=1
    fi
    (
        cd "$HOME/openocd-rpi-src"
        ./bootstrap
        ./configure --enable-linuxgpiod --enable-cmsis-dap
        make -j"$(nproc)"
        sudo make install
    )
fi

echo "==> [4/6] Setting environment variables in ~/.bashrc"
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

echo "==> [5/6] Adding user to dialout/gpio groups (for serial + GPIO access)"
sudo usermod -a -G dialout,gpio "$USER" || true

echo "==> [6/6] Installing VS Code extensions (skipped if 'code' CLI not found)"
if command -v code >/dev/null 2>&1; then
    code --install-extension raspberry-pi.raspberry-pi-pico || true
    code --install-extension ms-vscode.cpptools || true
    code --install-extension ms-vscode.cmake-tools || true
    code --install-extension marus25.cortex-debug || true
else
    echo "    'code' CLI not found — install extensions manually, see docs/03-vscode-setup.md"
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
