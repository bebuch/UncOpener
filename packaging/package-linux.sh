#!/bin/bash
# Linux AppImage packaging script for UncOpener
# Usage: ./packaging/package-linux.sh
#
# Prerequisites:
#   - CMake build completed with Release configuration
#   - linuxdeploy and linuxdeploy-plugin-qt (downloaded automatically if missing)
#
# For maximum compatibility, consider running this in a Docker container
# targeting an older Linux distribution (e.g., Ubuntu 18.04 or 20.04).

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERSION="1.0"
APP_NAME="UncOpener"
BUILD_DIR="${PROJECT_ROOT}/build/rel"
OUTPUT_DIR="${PROJECT_ROOT}/dist"
APPDIR="${OUTPUT_DIR}/AppDir"

echo "=== ${APP_NAME} Linux AppImage Packaging ==="
echo "Version: ${VERSION}"
echo "Build directory: ${BUILD_DIR}"
echo "Output directory: ${OUTPUT_DIR}"
echo ""

# Check for the executable
EXECUTABLE="${BUILD_DIR}/src/app/uncopener"
if [ ! -f "$EXECUTABLE" ]; then
    echo "ERROR: Cannot find uncopener executable at ${EXECUTABLE}"
    echo "Make sure you have built the project with Release configuration:"
    echo "  cmake --preset rel"
    echo "  cmake --build --preset rel"
    exit 1
fi

echo "Found executable: ${EXECUTABLE}"

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Create tools directory for linuxdeploy (separate from output to avoid uploading tools as artifacts)
TOOLS_DIR="${PROJECT_ROOT}/build/tools"
mkdir -p "${TOOLS_DIR}"

# Download linuxdeploy if not present
LINUXDEPLOY="${TOOLS_DIR}/linuxdeploy-x86_64.AppImage"
if [ ! -f "$LINUXDEPLOY" ]; then
    echo ""
    echo "Downloading linuxdeploy..."
    wget -q --show-progress -O "$LINUXDEPLOY" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY"
fi

# Download linuxdeploy Qt plugin if not present
LINUXDEPLOY_QT="${TOOLS_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"
if [ ! -f "$LINUXDEPLOY_QT" ]; then
    echo ""
    echo "Downloading linuxdeploy Qt plugin..."
    wget -q --show-progress -O "$LINUXDEPLOY_QT" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$LINUXDEPLOY_QT"
fi

# Clean up any existing AppDir
if [ -d "$APPDIR" ]; then
    echo "Removing existing AppDir..."
    rm -rf "$APPDIR"
fi

# Create .desktop file
echo ""
echo "Creating .desktop file..."
mkdir -p "${OUTPUT_DIR}"

cat > "${OUTPUT_DIR}/uncopener.desktop" << EOF
[Desktop Entry]
Type=Application
Name=UncOpener
Comment=Open UNC paths from custom URL scheme
Exec=uncopener %u
Icon=uncopener
Terminal=false
Categories=Utility;Network;
MimeType=x-scheme-handler/unc;
EOF

# Copy icon (rename to match Icon= entry in desktop file)
ICON_ORIG="${PROJECT_ROOT}/assets/linux/icon-256.png"
ICON_SRC="${OUTPUT_DIR}/uncopener.png"
if [ -f "$ICON_ORIG" ]; then
    cp "$ICON_ORIG" "$ICON_SRC"
else
    echo "WARNING: Icon not found at ${ICON_ORIG}"
    ICON_SRC="${PROJECT_ROOT}/assets/icon.svg"
fi

# Set Qt environment if not already set
if [ -z "$QMAKE" ]; then
    QMAKE=$(which qmake6 2>/dev/null || which qmake 2>/dev/null || echo "")
fi

if [ -n "$QMAKE" ]; then
    export QT_SELECT=qt6
fi

echo ""
echo "Building AppImage..."

# Run linuxdeploy
export LDAI_UPDATE_INFORMATION="gh-releases-zsync|bebuch|UncOpener|latest|${APP_NAME}-*-x86_64.AppImage.zsync"
export OUTPUT="${OUTPUT_DIR}/${APP_NAME}-${VERSION}-x86_64.AppImage"

"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$EXECUTABLE" \
    --desktop-file "${OUTPUT_DIR}/uncopener.desktop" \
    --icon-file "$ICON_SRC" \
    --plugin qt \
    --output appimage

echo ""
echo "=== Packaging complete ==="
echo "Output: ${OUTPUT}"
echo ""

# Show file info
ls -lh "$OUTPUT"

# Create a README for the AppImage
cat > "${OUTPUT_DIR}/README-AppImage.txt" << EOF
# ${APP_NAME} ${VERSION} - Linux AppImage

## Installation

1. Make the AppImage executable:
   chmod +x ${APP_NAME}-${VERSION}-x86_64.AppImage

2. Run the AppImage:
   ./${APP_NAME}-${VERSION}-x86_64.AppImage

## Desktop Integration

To integrate with your desktop environment:
1. Move the AppImage to ~/.local/bin/ or /opt/
2. Run UncOpener and use the GUI to register the URL scheme

The registration creates a .desktop file in ~/.local/share/applications/
and uses xdg-mime to set the default handler.

## Usage

Once registered, clicking links like 'uncopener://server/path/to/resource' in your browser
will open the corresponding SMB path in your file manager.

## System Requirements

- Linux x86_64
- FUSE support (for running AppImages)
- A desktop environment with SMB/CIFS support

## More Information

https://github.com/bebuch/UncOpener
EOF

echo "Created README at ${OUTPUT_DIR}/README-AppImage.txt"
