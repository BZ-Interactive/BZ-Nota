#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

declare -A TARGETS=(
    ["1"]="x86_64"
    ["2"]="aarch64"
    ["3"]="arm"
    ["4"]="i386"
)

print_target() {
    echo "  [$1] $2"
}

echo "========================================"
echo "  BZ-Nota System-Wide Installation"
echo "========================================"
echo ""
echo "Install prefix: $INSTALL_PREFIX"
echo ""
echo "Available targets (select one to install):"
for key in $(echo "${!TARGETS[@]}" | tr ' ' '\n' | sort | tr '\n' ' '); do
    for k in $key; do
        print_target "$k" "${TARGETS[$k]}"
    done
done
echo ""
read -p "Select target to install [default: 1 for x86_64]: " selection
selection="${selection:-1}"

arch="${TARGETS[$selection]:-$selection}"
binary_path="$BUILD_DIR/types/$arch/bznota"

if [[ ! -f "$binary_path" ]]; then
    echo "Error: Binary not found at $binary_path"
    echo "Please run build.sh first to build the selected target."
    exit 1
fi

if [[ $EUID -ne 0 ]]; then
    echo "This script requires root privileges."
    echo "Run with: sudo $0"
    exit 1
fi

echo ""
echo "Installing bznota ($arch) to $INSTALL_PREFIX..."
echo ""

install -Dm755 "$binary_path" "$INSTALL_PREFIX/bin/bznota"

install -Dm644 "$BUILD_DIR/$arch/bznota.desktop" "$INSTALL_PREFIX/share/applications/bznota.desktop"

install -d "$INSTALL_PREFIX/share/icons/hicolor/16x16/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/24x24/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/32x32/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/48x48/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/64x64/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/128x128/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/512x512/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/1024x1024/apps"
install -d "$INSTALL_PREFIX/share/icons/hicolor/scalable/apps"

install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_16.png" "$INSTALL_PREFIX/share/icons/hicolor/16x16/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_24.png" "$INSTALL_PREFIX/share/icons/hicolor/24x24/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_32.png" "$INSTALL_PREFIX/share/icons/hicolor/32x32/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_48.png" "$INSTALL_PREFIX/share/icons/hicolor/48x48/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_64.png" "$INSTALL_PREFIX/share/icons/hicolor/64x64/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_128.png" "$INSTALL_PREFIX/share/icons/hicolor/128x128/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_256.png" "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_512.png" "$INSTALL_PREFIX/share/icons/hicolor/512x512/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_1024.png" "$INSTALL_PREFIX/share/icons/hicolor/1024x1024/apps/bznota.png"
install -Dm644 "$SCRIPT_DIR/assets/BZ-Nota_icon_scalable.svg" "$INSTALL_PREFIX/share/icons/hicolor/scalable/apps/bznota.svg"

install -d "$INSTALL_PREFIX/share/bznota"
install -Dm644 "$BUILD_DIR/types/$arch/share/bznota/LICENSE" "$INSTALL_PREFIX/share/bznota/LICENSE"
install -Dm644 "$BUILD_DIR/types/$arch/share/bznota/logo_raw.txt" "$INSTALL_PREFIX/share/bznota/logo_raw.txt"

install -d "$INSTALL_PREFIX/share/licenses/bznota"
install -Dm644 "$BUILD_DIR/types/$arch/share/bznota/LICENSE" "$INSTALL_PREFIX/share/licenses/bznota/LICENSE"

if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$INSTALL_PREFIX/share/icons/hicolor" 2>/dev/null || true
fi

if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$INSTALL_PREFIX/share/applications" 2>/dev/null || true
fi

echo ""
echo "========================================"
echo "Installation complete!"
echo ""
echo "Binary: $INSTALL_PREFIX/bin/bznota"
echo "Desktop: $INSTALL_PREFIX/share/applications/bznota.desktop"
echo "Icons: $INSTALL_PREFIX/share/icons/hicolor/*/apps/bznota.*"
echo "Data: $INSTALL_PREFIX/share/bznota/"
echo "License: $INSTALL_PREFIX/share/licenses/bznota/LICENSE"
echo "========================================"
