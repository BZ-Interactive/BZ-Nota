#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
RELEASE_DIR="$SCRIPT_DIR/release"

# Get path from file if it exists, otherwise fall back to environment variable
if [[ -f "$SCRIPT_DIR/zig-dir.txt" ]]; then
    ZIG_DIR=$(head -n 1 "$SCRIPT_DIR/zig-dir.txt" | xargs)
fi

# Final validation
if [[ -z "$ZIG_DIR" || ! -x "$ZIG_DIR/zig" ]]; then
    # Final check: is it already in the system PATH?
    if ! command -v zig &> /dev/null; then
        echo "Error: Zig not found."
        echo "Please ensure ZIG_DIR is set or $SCRIPT_DIR/zig-dir.txt exists with a valid path."
        exit 1
    fi
else
    export PATH="$ZIG_DIR:$PATH"
fi

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

declare -A TOOLCHAIN_MAP=(
    ["x86_64"]="x86_64-linux-musl"
    ["aarch64"]="aarch64-linux-musl"
    ["arm"]="arm-linux-musleabihf"
    ["i386"]="x86-linux-musl"
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
          -DCMAKE_TOOLCHAIN_FILE="$SCRIPT_DIR/cmake/zig-toolchains/zig-toolchain-${TOOLCHAIN_MAP[$arch]}.cmake" \
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

    # Step 2.5: Copy existing install/uninstall scripts from root
    echo "  Adding installation scripts..."
    cp "$SCRIPT_DIR/install.sh" "$arch_dir/"
    cp "$SCRIPT_DIR/uninstall.sh" "$arch_dir/"
    
    # Ensure they stay executable in the release package
    chmod +x "$arch_dir/install.sh" "$arch_dir/uninstall.sh"
done

echo ""
echo "Step 3: Creating tar.gz archives..."
echo ""

for arch in x86_64 aarch64 arm i386; do
    ARCH_NAME="bznota_v${VERSION}_${ARCH_MAP[$arch]}"
    ARCHIVE_NAME="${ARCH_NAME}.tar.gz"

    echo "  Creating $ARCHIVE_NAME..."
    tar -czf "$RELEASE_DIR/$ARCHIVE_NAME" -C "$RELEASE_DIR" "$ARCH_NAME"
    echo "    Size: $(du -h "$RELEASE_DIR/$ARCHIVE_NAME" | cut -f1)"
done

echo ""
echo "========================================"
echo "Releases created in: $RELEASE_DIR"
echo ""
echo "Contents:"
ls -lah "$RELEASE_DIR"
echo ""
echo "========================================"
echo ""
echo "Upload these to GitHub Releases:"
for arch in x86_64 aarch64 arm i386; do
    echo "  bznota_v${VERSION}_${ARCH_MAP[$arch]}.tar.gz"
done
echo ""
