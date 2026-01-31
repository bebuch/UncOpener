#!/bin/bash
# Generate platform-specific icon assets from icon.svg
# Usage: ./tools/generate-icons.sh
#
# Prerequisites:
#   - inkscape or rsvg-convert (SVG to PNG conversion)
#   - ImageMagick (PNG to ICO conversion)
#   - optipng (PNG optimization, optional)

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ASSETS_DIR="$PROJECT_ROOT/assets"
SVG_SOURCE="$PROJECT_ROOT/icon.svg"

# Icon sizes for different purposes
LINUX_SIZES="16 24 32 48 64 128 256 512"
WINDOWS_SIZES="16 24 32 48 64 128 256"

echo "=== UncOpener Icon Generator ==="
echo "Source: $SVG_SOURCE"
echo "Output: $ASSETS_DIR"
echo ""

# Check for required tools
check_tool() {
    if command -v "$1" &> /dev/null; then
        echo "[OK] $1 found: $(command -v "$1")"
        return 0
    else
        echo "[MISSING] $1 not found"
        return 1
    fi
}

echo "Checking required tools..."
SVG_CONVERTER=""
if check_tool "inkscape"; then
    SVG_CONVERTER="inkscape"
elif check_tool "rsvg-convert"; then
    SVG_CONVERTER="rsvg-convert"
else
    echo "ERROR: Neither inkscape nor rsvg-convert found. Please install one of them."
    exit 1
fi

HAS_MAGICK=false
if check_tool "convert"; then
    HAS_MAGICK=true
fi

HAS_OPTIPNG=false
if check_tool "optipng"; then
    HAS_OPTIPNG=true
    echo ""
    echo "optipng version info:"
    optipng --help 2>&1 | head -5
fi
echo ""

# Create assets directory
mkdir -p "$ASSETS_DIR/linux"
mkdir -p "$ASSETS_DIR/windows"

# Function to convert SVG to PNG
svg_to_png() {
    local size=$1
    local output=$2

    if [ "$SVG_CONVERTER" = "inkscape" ]; then
        inkscape --export-type=png --export-filename="$output" -w "$size" -h "$size" "$SVG_SOURCE" 2>/dev/null
    else
        rsvg-convert -w "$size" -h "$size" "$SVG_SOURCE" -o "$output"
    fi
}

# Generate Linux PNG icons
echo "Generating Linux PNG icons..."
for size in $LINUX_SIZES; do
    output="$ASSETS_DIR/linux/icon-${size}.png"
    echo "  Creating ${size}x${size} PNG..."
    svg_to_png "$size" "$output"

    if [ "$HAS_OPTIPNG" = true ]; then
        optipng -o7 -quiet "$output"
    fi
done

# Generate Windows ICO
if [ "$HAS_MAGICK" = true ]; then
    echo ""
    echo "Generating Windows ICO..."

    # Create temporary PNGs for ICO
    TEMP_PNGS=""
    for size in $WINDOWS_SIZES; do
        temp_png="/tmp/uncopener-icon-${size}.png"
        svg_to_png "$size" "$temp_png"
        TEMP_PNGS="$TEMP_PNGS $temp_png"
    done

    # Create ICO from all sizes
    convert $TEMP_PNGS "$ASSETS_DIR/windows/icon.ico"
    echo "  Created icon.ico with sizes: $WINDOWS_SIZES"

    # Clean up temp files
    rm -f $TEMP_PNGS
else
    echo ""
    echo "WARNING: ImageMagick not found, skipping Windows ICO generation"
fi

# Copy SVG for in-app use (it's already there but let's make an assets copy)
cp "$SVG_SOURCE" "$ASSETS_DIR/icon.svg"

echo ""
echo "=== Icon generation complete ==="
echo ""
echo "Generated files:"
ls -la "$ASSETS_DIR/"
echo ""
ls -la "$ASSETS_DIR/linux/"
if [ -d "$ASSETS_DIR/windows" ]; then
    echo ""
    ls -la "$ASSETS_DIR/windows/"
fi
