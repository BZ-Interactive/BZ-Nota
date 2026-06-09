#!/bin/bash
set -e

BOLD='\e[1m'
RESET='\e[0m'
GREEN='\e[0;32m'
YELLOW='\e[0;33m'
RED='\e[0;31m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

if [[ $EUID -ne 0 ]]; then
    echo "Elevating permissions to install to $INSTALL_PREFIX..."
    exec sudo env INSTALL_PREFIX="$INSTALL_PREFIX" "$0" "$@"
fi

echo "========================================"
echo -e "${BOLD}  BZ-Nota System-Wide Installer${RESET}"
echo ""
echo "Install prefix: $INSTALL_PREFIX"

if [[ ! -f "$SCRIPT_DIR/bznota" ]]; then
    echo -e "${RED}Error: 'bznota' binary not found in $SCRIPT_DIR${RESET}"
    exit 1
fi

SYS_ARCH="$(uname -m)"
BINARY_INFO=$(file -b "$SCRIPT_DIR/bznota" 2>/dev/null || echo "")
DETECTED_ARCH="unknown"

if [[ -n "$BINARY_INFO" ]]; then
    if [[ "$BINARY_INFO" == *"x86-64"* ]]; then DETECTED_ARCH="x86_64"; fi
    if [[ "$BINARY_INFO" == *"aarch64"* ]]; then DETECTED_ARCH="aarch64"; fi
    if [[ "$BINARY_INFO" == *"ARM"* ]]; then DETECTED_ARCH="arm"; fi
    if [[ "$BINARY_INFO" == *"Intel 80386"* ]]; then DETECTED_ARCH="i386"; fi
fi

if [[ "$DETECTED_ARCH" != "unknown" ]]; then
    # Standardize arm64 naming deviations
    NORM_SYS="$SYS_ARCH"
    [[ "$SYS_ARCH" == "arm64" ]] && NORM_SYS="aarch64"
    [[ "$SYS_ARCH" == "armv7l" || "$SYS_ARCH" == "armhf" ]] && NORM_SYS="arm"
    [[ "$SYS_ARCH" == "i686" ]] && NORM_SYS="i386"

    if [[ "$NORM_SYS" != "$DETECTED_ARCH" ]]; then
        echo -e "${RED}❌ Error: Architecture Mismatch!${RESET}"
        echo "Your machine is running: $SYS_ARCH"
        echo "This package contains a binary built for: $DETECTED_ARCH"
        echo "Aborting installation."
        exit 1
    fi
    echo -e "${GREEN}Architecture check passed: $DETECTED_ARCH${RESET}"
else
    echo -e "${YELLOW} Warning: Could not read binary metadata. Proceeding...${RESET}"
fi

echo "Installing files..."

# Create system directories
mkdir -p "$INSTALL_PREFIX/bin"
mkdir -p "$INSTALL_PREFIX/share/applications"
mkdir -p "$INSTALL_PREFIX/share/bznota"
mkdir -p "$INSTALL_PREFIX/share/licenses/bznota"

# Install Assets
install -m755 "$SCRIPT_DIR/bznota" "$INSTALL_PREFIX/bin/bznota"
[[ -f "$SCRIPT_DIR/bznota.desktop" ]] && install -m644 "$SCRIPT_DIR/bznota.desktop" "$INSTALL_PREFIX/share/applications/bznota.desktop"
[[ -f "$SCRIPT_DIR/LICENSE" ]] && install -m644 "$SCRIPT_DIR/LICENSE" "$INSTALL_PREFIX/share/bznota/LICENSE"
[[ -f "$SCRIPT_DIR/LICENSE" ]] && install -m644 "$SCRIPT_DIR/LICENSE" "$INSTALL_PREFIX/share/licenses/bznota/LICENSE"
[[ -f "$SCRIPT_DIR/BZ-Nota_Logo.ansi" ]] && install -m644 "$SCRIPT_DIR/BZ-Nota_Logo.ansi" "$INSTALL_PREFIX/share/bznota/BZ-Nota_Logo.ansi"

# Install Icons
if [[ -d "$SCRIPT_DIR/assets" ]]; then
    install -d "$INSTALL_PREFIX/share/icons/hicolor"/{16x16,24x24,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024,scalable}/apps
    for size in 16 24 32 48 64 128 256 512 1024; do
        if [[ -f "$SCRIPT_DIR/assets/BZ-Nota_icon_${size}.png" ]]; then
            install -m644 "$SCRIPT_DIR/assets/BZ-Nota_icon_${size}.png" "$INSTALL_PREFIX/share/icons/hicolor/${size}x${size}/apps/bznota.png" 2>/dev/null || true
        fi
    done
    [[ -f "$SCRIPT_DIR/assets/BZ-Nota_icon_scalable.svg" ]] && install -m644 "$SCRIPT_DIR/assets/BZ-Nota_icon_scalable.svg" "$INSTALL_PREFIX/share/icons/hicolor/scalable/apps/bznota.svg"
fi

# Refresh Indexing Databases
command -v gtk-update-icon-cache &> /dev/null && gtk-update-icon-cache -f -t "$INSTALL_PREFIX/share/icons/hicolor" 2>/dev/null || true
command -v update-desktop-database &> /dev/null && update-desktop-database "$INSTALL_PREFIX/share/applications" 2>/dev/null || true

echo ""
echo "========================================"
echo -e "${GREEN}${BOLD}Installation complete!${RESET}"
echo "========================================"
