#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ZIG_DIR="${ZIG_DIR:-/home/barkin/Workspace/Tools/zig-x86_64-linux-0.15.2}"
TOOLCHAIN_DIR="$SCRIPT_DIR/cmake/zig-toolchains"
BUILD_DIR="$SCRIPT_DIR/build"

declare -A TARGETS=(
    ["1"]="x86_64"
    ["2"]="aarch64"
    ["3"]="arm"
    ["4"]="i386"
)

declare -A TOOLCHAINS=(
    ["x86_64"]="zig-toolchain-x86_64-linux-musl.cmake"
    ["aarch64"]="zig-toolchain-aarch64-linux-musl.cmake"
    ["arm"]="zig-toolchain-arm-linux-musleabihf.cmake"
    ["i386"]="zig-toolchain-x86-linux-musl.cmake"
)

print_target() {
    echo "  [$1] $2"
}

echo "========================================"
echo "  BZ-Nota Cross-Compilation Build Script"
echo "========================================"
echo ""
echo "Available targets:"
for key in "$(echo "${!TARGETS[@]}" | tr ' ' '\n' | sort | tr '\n' ' ')"; do
    for k in $key; do
        print_target "$k" "${TARGETS[$k]}"
    done
done
echo "  [a] All targets"
echo "  [q] Quit"
echo ""

read -p "Select target(s) [default: 1 for x86_64]: " selection
selection="${selection:-1}"

if [[ "$selection" == "q" ]]; then
    echo "Aborted."
    exit 0
fi

if [[ "$selection" == "a" ]]; then
    selected_targets=("${TARGETS[@]}")
else
    IFS=',' read -ra selected_targets <<< "$selection"
fi

echo ""
echo "Building targets: ${selected_targets[*]}"
echo ""

mkdir -p "$BUILD_DIR"

for target in "${selected_targets[@]}"; do
    arch="${TARGETS[$target]:-$target}"
    if [[ -z "${TOOLCHAINS[$arch]}" ]]; then
        echo "Error: Unknown target '$target'"
        continue
    fi
    
    toolchain="${TOOLCHAINS[$arch]}"
    output_dir="$BUILD_DIR/types/$arch"
    build_dir="$BUILD_DIR/$arch"
    
    echo "----------------------------------------"
    echo "Building $arch..."
    echo "  Toolchain: $toolchain"
    echo "  Output: $output_dir"
    echo "----------------------------------------"
    
    rm -rf "$build_dir"
    mkdir -p "$build_dir"
    
    cmake -S "$SCRIPT_DIR" \
          -B "$build_dir" \
          -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAINS_DIR/$toolchain" \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX="$output_dir"
    
    cmake --build "$build_dir" --target bznota -j"$(nproc)"
    
    mkdir -p "$output_dir"
    cp "$build_dir/bznota" "$output_dir/"
    cp "$build_dir/bznota.desktop" "$output_dir/" 2>/dev/null || true
    
    mkdir -p "$output_dir/share/bznota"
    cp "$build_dir/LICENSE" "$output_dir/share/bznota/" 2>/dev/null || true
    cp "$build_dir/logo_raw.txt" "$output_dir/share/bznota/" 2>/dev/null || true
    
    echo ""
    echo "Done: $output_dir/bznota"
    echo ""
done

echo "========================================"
echo "Build complete!"
echo "Binaries location: $BUILD_DIR/types/"
echo "========================================"
