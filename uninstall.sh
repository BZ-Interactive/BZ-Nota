#!/bin/bash
set -e

INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"

echo "========================================"
echo "  BZ-Nota Uninstallation"
echo "========================================"
echo ""
echo "Install prefix: $INSTALL_PREFIX"
echo ""

if [[ $EUID -ne 0 ]]; then
    echo "This script requires root privileges."
    echo "Run with: sudo $0"
    exit 1
fi

echo "Removing files..."

rm -f "$INSTALL_PREFIX/bin/bznota"

rm -f "$INSTALL_PREFIX/share/applications/bznota.desktop"

rm -f "$INSTALL_PREFIX/share/icons/hicolor/16x16/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/24x24/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/32x32/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/48x48/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/64x64/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/128x128/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/512x512/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/1024x1024/apps/bznota.png"
rm -f "$INSTALL_PREFIX/share/icons/hicolor/scalable/apps/bznota.svg"

rm -rf "$INSTALL_PREFIX/share/bznota"
rm -rf "$INSTALL_PREFIX/share/licenses/bznota"

rmdir "$INSTALL_PREFIX/share/icons/hicolor/16x16/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/24x24/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/32x32/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/48x48/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/64x64/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/128x128/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/256x256/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/512x512/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/1024x1024/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/scalable/apps" 2>/dev/null || true
rmdir "$INSTALL_PREFIX/share/icons/hicolor/scalable" 2>/dev/null || true

if command -v gtk-update-icon-cache &> /dev/null; then
    gtk-update-icon-cache -f -t "$INSTALL_PREFIX/share/icons/hicolor" 2>/dev/null || true
fi

if command -v update-desktop-database &> /dev/null; then
    update-desktop-database "$INSTALL_PREFIX/share/applications" 2>/dev/null || true
fi

echo ""
echo "========================================"
echo "Uninstallation complete!"
echo "========================================"
