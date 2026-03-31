#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
RELEASE_DIR="$SCRIPT_DIR/release"

VERSION=$(grep -oP 'VERSION \K[0-9]+\.[0-9]+\.[0-9]+' "$SCRIPT_DIR/CMakeLists.txt")

declare -A ARCH_MAP=(
    ["x86_64"]="x86_64"
    ["aarch64"]="aarch64_arm64"
    ["arm"]="arm32"
    ["i386"]="i386"
)

declare -A ARCH_DESC=(
    ["x86_64"]="64-bit x86 (Intel/AMD)"
    ["aarch64"]="64-bit ARM (arm64)"
    ["arm"]="32-bit ARM (arm32)"
    ["i386"]="32-bit x86"
)

echo "========================================"
echo "  BZ-Nota Release Builder"
echo "  Version: v$VERSION"
echo "========================================"
echo ""
echo "Architectures:"
for arch in x86_64 aarch64 arm i386; do
    echo "  - $arch: ${ARCH_DESC[$arch]}"
done
echo ""

echo "Step 1: Building all targets..."
echo ""

for arch in x86_64 aarch64 arm i386; do
    echo "Building ${ARCH_DESC[$arch]} ($arch)..."
    rm -rf "$BUILD_DIR/$arch"
    mkdir -p "$BUILD_DIR/$arch"
    
    cmake -S "$SCRIPT_DIR" \
          -B "$BUILD_DIR/$arch" \
          -DCMAKE_TOOLCHAIN_FILE="$SCRIPT_DIR/cmake/zig-toolchains/zig-toolchain-${arch}-linux-musl.cmake" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$BUILD_DIR/types/$arch" 2>&1 | grep -v "FTXUI\|option\|CMP0077" || true
    
    cmake --build "$BUILD_DIR/$arch" --target bznota -j"$(nproc)" 2>&1 | tail -1
    echo ""
done

echo "Step 2: Creating release directories..."
echo ""

for arch in x86_64 aarch64 arm i386; do
    ARCH_NAME="bznota_v${VERSION}_${ARCH_MAP[$arch]}"
    arch_dir="$RELEASE_DIR/$ARCH_NAME"
    
    echo "  Creating $ARCH_NAME (${ARCH_DESC[$arch]})..."
    
    mkdir -p "$arch_dir/share/bznota"
    mkdir -p "$arch_dir/share/icons/hicolor"
    mkdir -p "$arch_dir/share/licenses/bznota"
    
    cp "$BUILD_DIR/$arch/bznota" "$arch_dir/"
    cp "$BUILD_DIR/$arch/bznota.desktop" "$arch_dir/"
    
    cp "$BUILD_DIR/$arch/LICENSE" "$arch_dir/share/bznota/"
    cp "$BUILD_DIR/$arch/logo_raw.txt" "$arch_dir/share/bznota/"
    cp "$BUILD_DIR/$arch/LICENSE" "$arch_dir/share/licenses/bznota/"
    
    for size in 16 24 32 48 64 128 256 512 1024; do
        mkdir -p "$arch_dir/share/icons/hicolor/${size}x${size}/apps"
        cp "$SCRIPT_DIR/assets/BZ-Nota_icon_${size}.png" "$arch_dir/share/icons/hicolor/${size}x${size}/apps/bznota.png"
    done
    
    mkdir -p "$arch_dir/share/icons/hicolor/scalable/apps"
    cp "$SCRIPT_DIR/assets/BZ-Nota_icon_scalable.svg" "$arch_dir/share/icons/hicolor/scalable/apps/bznota.svg"
    
    cat > "$arch_dir/install.sh" << 'INSTALLSCRIPT'
#!/bin/bash
set -e

echo "Installing BZ-Nota to ~/.local..."
echo ""

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
HOME_DIR="$HOME"

echo "  Installing binary..."
mkdir -p "$HOME_DIR/.local/bin"
install -Dm755 "$SCRIPT_DIR/bznota" "$HOME_DIR/.local/bin/bznota"

echo "  Adding ~/.local/bin to PATH if needed..."
if ! grep -q 'export PATH=.*\.local/bin' "$HOME_DIR/.bashrc" 2>/dev/null; then
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$HOME_DIR/.bashrc"
    echo "  Added ~/.local/bin to PATH in ~/.bashrc"
fi

echo "  Installing desktop file..."
mkdir -p "$HOME_DIR/.local/share/applications"
install -Dm644 "$SCRIPT_DIR/bznota.desktop" "$HOME_DIR/.local/share/applications/bznota.desktop"

echo "  Installing icons..."
mkdir -p "$HOME_DIR/.local/share/icons/hicolor"
cp -r "$SCRIPT_DIR/share/icons/"* "$HOME_DIR/.local/share/icons/"

echo "  Installing data files..."
mkdir -p "$HOME_DIR/.local/share/bznota"
cp -r "$SCRIPT_DIR/share/bznota/"* "$HOME_DIR/.local/share/bznota/"
mkdir -p "$HOME_DIR/.local/share/licenses/bznota"
cp -r "$SCRIPT_DIR/share/licenses/"* "$HOME_DIR/.local/share/licenses/"

echo "  Updating caches..."
gtk-update-icon-cache -f -t "$HOME_DIR/.local/share/icons/hicolor" 2>/dev/null || true
update-desktop-database "$HOME_DIR/.local/share/applications" 2>/dev/null || true

echo ""
echo "Done!"
echo ""
echo "Start bznota by running:"
echo "  ~/.local/bin/bznota"
echo ""
echo "Or restart your terminal and run:"
echo "  bznota"
INSTALLSCRIPT

    cat > "$arch_dir/uninstall.sh" << 'UNINSTALLSCRIPT'
#!/bin/bash
set -e

echo "Uninstalling BZ-Nota..."
echo ""

HOME_DIR="$HOME"

echo "  Removing binary..."
rm -f "$HOME_DIR/.local/bin/bznota"
rmdir "$HOME_DIR/.local/bin" 2>/dev/null || true

echo "  Removing desktop file..."
rm -f "$HOME_DIR/.local/share/applications/bznota.desktop"
rmdir "$HOME_DIR/.local/share/applications" 2>/dev/null || true

echo "  Removing icons..."
for size in 16 24 32 48 64 128 256 512 1024; do
    rm -f "$HOME_DIR/.local/share/icons/hicolor/${size}x${size}/apps/bznota.png" 2>/dev/null || true
done
rm -f "$HOME_DIR/.local/share/icons/hicolor/scalable/apps/bznota.svg" 2>/dev/null || true

echo "  Removing data files..."
rm -rf "$HOME_DIR/.local/share/bznota"
rm -rf "$HOME_DIR/.local/share/licenses/bznota"

echo "  Updating caches..."
gtk-update-icon-cache -f -t "$HOME_DIR/.local/share/icons/hicolor" 2>/dev/null || true
update-desktop-database "$HOME_DIR/.local/share/applications" 2>/dev/null || true

echo ""
echo "Done!"
echo ""
echo "Note: ~/.bashrc was not modified. You can manually remove the"
echo "      PATH line if you no longer need it."
UNINSTALLSCRIPT

    chmod +x "$arch_dir/install.sh" "$arch_dir/uninstall.sh"
done

echo ""
echo "========================================"
echo "Releases created in: $RELEASE_DIR"
echo ""
echo "Contents:"
ls -la "$RELEASE_DIR"
echo ""
echo "========================================"
