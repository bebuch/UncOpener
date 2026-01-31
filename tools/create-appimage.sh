#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build"
APPDIR="${BUILD_DIR}/AppDir"

echo "=== UncOpener AppImage Builder ==="

# Check for required tools
check_tool() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed or not in PATH"
        return 1
    fi
}

# Download linuxdeploy if not available
LINUXDEPLOY="${BUILD_DIR}/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="${BUILD_DIR}/linuxdeploy-plugin-qt-x86_64.AppImage"

download_tools() {
    echo "Downloading linuxdeploy tools..."

    if [ ! -f "$LINUXDEPLOY" ]; then
        echo "  Downloading linuxdeploy..."
        wget -q -O "$LINUXDEPLOY" \
            "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
        chmod +x "$LINUXDEPLOY"
    fi

    if [ ! -f "$LINUXDEPLOY_QT" ]; then
        echo "  Downloading linuxdeploy-plugin-qt..."
        wget -q -O "$LINUXDEPLOY_QT" \
            "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
        chmod +x "$LINUXDEPLOY_QT"
    fi
}

# Build the project in Release mode
build_project() {
    echo "Building project in Release mode..."
    cmake -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DUNCOPENER_ENABLE_CLANG_TIDY=OFF "$PROJECT_DIR"
    cmake --build "$BUILD_DIR" --parallel
}

# Create AppImage
create_appimage() {
    echo "Creating AppImage..."

    # Clean previous AppDir
    rm -rf "$APPDIR"
    mkdir -p "$APPDIR"

    # Set environment for Qt plugin
    export QMAKE=$(which qmake6 2>/dev/null || which qmake)
    export QML_SOURCES_PATHS="${PROJECT_DIR}/src"

    # Copy and rename icon to match desktop file Icon= entry
    ICON_DIR="${APPDIR}/usr/share/icons/hicolor/scalable/apps"
    mkdir -p "$ICON_DIR"
    cp "${PROJECT_DIR}/assets/icon.svg" "${ICON_DIR}/uncopener.svg"

    # Run linuxdeploy
    # Skip AppStream metadata to avoid strict validation issues
    "$LINUXDEPLOY" \
        --appdir "$APPDIR" \
        --executable "${BUILD_DIR}/src/app/uncopener" \
        --desktop-file "${PROJECT_DIR}/data/org.uncopener.UncOpener.desktop" \
        --icon-file "${ICON_DIR}/uncopener.svg" \
        --plugin qt \
        --output appimage

    # Move AppImage to build directory
    mv UncOpener*.AppImage "$BUILD_DIR/" 2>/dev/null || true

    echo ""
    echo "=== AppImage created successfully ==="
    echo "Location: ${BUILD_DIR}/UncOpener-x86_64.AppImage"
}

# Main
cd "$PROJECT_DIR"

check_tool wget || exit 1
check_tool cmake || exit 1

download_tools
build_project
create_appimage
