#!/bin/bash
set -e

GITHUB_USER="BZ-Interactive"
GITHUB_REPO="BZ-Nota"
VERSION=$(curl -s "https://api.github.com/repos/BZ-Interactive/BZ-Nota/releases/latest" | grep -Po '"tag_name": "v\K[^"]*')

if [[ -z "$VERSION" ]]; then
    echo "Error: Could not fetch the latest version from GitHub."
    exit 1
fi

echo "========================================"
echo "  BZ-Nota Remote Installer"
echo "  Version: v$VERSION"
echo "========================================"

detect_arch() {
    case "$(uname -m)" in
        x86_64)           echo "x86_64" ;;
        aarch64|arm64)    echo "aarch64" ;;
        armv7l|armhf)     echo "arm"     ;;
        i686|i386)        echo "i386"    ;;
        *)
            echo "Error: Unsupported architecture: $(uname -m)" >&2
            exit 1
            ;;
    esac
}

ARCH="$(detect_arch)"
URL="https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/download/v$VERSION/bznota_v${VERSION}_${ARCH}.tar.gz"
TEMP_DIR=$(mktemp -d)

trap '[[ -d "$TEMP_DIR" ]] && rm -rf "$TEMP_DIR"' EXIT

echo "Downloading package for $ARCH..."
if ! curl -fsSL "$URL" -o "$TEMP_DIR/bznota.tar.gz"; then
    echo "Error: Failed to download package from GitHub."
    exit 1
fi

echo "Extracting archive..."
tar -xzf "$TEMP_DIR/bznota.tar.gz" -C "$TEMP_DIR"

EXTRACTED_DIR="$TEMP_DIR/bznota_v${VERSION}_${ARCH}"
if [[ ! -d "$EXTRACTED_DIR" ]]; then
    echo "Error: Extracted archive directory structure is mismatched." >&2
    exit 1
fi

cd "$EXTRACTED_DIR"
./install.sh
