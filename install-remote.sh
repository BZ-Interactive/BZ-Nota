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
echo "  Version: $VERSION"
echo "========================================"
echo ""

detect_arch() {
    case "$(uname -m)" in
        x86_64)
            echo "x86_64"
            ;;
        aarch64|arm64)
            echo "aarch64_arm64"
            ;;
        armv7l|armhf)
            echo "arm32"
            ;;
        i686|i386)
            echo "i386"
            ;;
        *)
            echo "Error: Unsupported architecture: $(uname -m)" >&2
            exit 1
            ;;
    esac
}

ARCH="$(detect_arch)"
ARCH_LABEL="$(uname -m)"
URL="https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/download/v$VERSION/bznota_v${VERSION}_${ARCH}.tar.gz"
TEMP_DIR=$(mktemp -d)

echo "Detected architecture: $ARCH_LABEL"
echo "Downloading: bznota_v${VERSION}_${ARCH}.tar.gz"
echo ""

cleanup() {
    if [[ -d "$TEMP_DIR" ]]; then
        rm -rf "$TEMP_DIR"
    fi
}
trap cleanup EXIT

echo "Downloading..."
if ! curl -fsSL "$URL" -o "$TEMP_DIR/bznota.tar.gz"; then
    echo ""
    echo "Error: Failed to download from:"
    echo "  $URL"
    echo ""
    echo "Make sure the release is published at:"
    echo "  https://github.com/$GITHUB_USER/$GITHUB_REPO/releases/tag/v$VERSION"
    exit 1
fi

echo "Extracting..."
tar -xzf "$TEMP_DIR/bznota.tar.gz" -C "$TEMP_DIR"

EXTRACTED_DIR="$TEMP_DIR/bznota_v${VERSION}_${ARCH}"

if [[ ! -d "$EXTRACTED_DIR" ]]; then
    echo "Error: Extracted directory not found" >&2
    exit 1
fi

echo ""
echo "Running installer..."
echo ""

cd "$EXTRACTED_DIR"
./install.sh
