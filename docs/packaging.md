# Packaging Guide

## Overview

UncOpener provides platform-specific packaging for easy distribution:
- **Windows**: Portable ZIP with Qt runtime
- **Linux**: AppImage with bundled dependencies

## Release Artifact Naming

Artifacts follow this naming convention:
```
UncOpener-<version>-<platform>.<ext>
```

Examples:
- `UncOpener-0.1.0-windows.zip`
- `UncOpener-0.1.0-x86_64.AppImage`

## Windows Packaging

### Prerequisites
- Qt6 installed with `windeployqt` available
- PowerShell 5.1 or later
- CMake Release build completed

### Building

```powershell
# Build in Release mode
cmake --preset rel
cmake --build --preset rel

# Create ZIP package
powershell -ExecutionPolicy Bypass -File packaging/package-windows.ps1
```

### Output
The script creates:
- `dist/UncOpener-<version>-windows.zip` - Distributable ZIP
- `dist/UncOpener-<version>-windows/` - Staging directory

### Portable Installation Notes
The Windows package is portable - no installer required:
1. Extract ZIP to any location
2. Run `uncopener.exe`
3. Use the GUI to register the URL scheme

Scheme registration is per-user (HKEY_CURRENT_USER) and works without admin rights.

## Linux Packaging

### Prerequisites
- Qt6 development packages
- `wget` for downloading linuxdeploy
- FUSE support (for running AppImages)
- CMake Release build completed

### Building

```bash
# Build in Release mode
cmake --preset rel
cmake --build --preset rel

# Create AppImage
./packaging/package-linux.sh
```

### Docker Build (Recommended for Distribution)

For maximum compatibility across Linux distributions, build in Docker:

```bash
# Build the Docker image
docker build -t uncopener-builder -f packaging/Dockerfile.linux .

# Run the build
docker run -v $(pwd):/src -v $(pwd)/dist:/dist uncopener-builder
```

This uses Ubuntu 20.04 as a base, ensuring compatibility with distributions
from that era and newer.

### Output
The script creates:
- `dist/UncOpener-<version>-x86_64.AppImage` - Distributable AppImage
- `dist/README-AppImage.txt` - Usage instructions

### AppImage Usage
```bash
# Make executable
chmod +x UncOpener-0.1.0-x86_64.AppImage

# Run
./UncOpener-0.1.0-x86_64.AppImage
```

## Reproducible Builds

For reproducible builds:

1. **Use fixed toolchain versions** - Pin compiler and Qt versions
2. **Use Docker for Linux** - The provided Dockerfile ensures consistent environment
3. **Document dependencies** - Record exact versions of all tools used
4. **Timestamp normalization** - Consider using `SOURCE_DATE_EPOCH` for reproducible timestamps

### Environment Variables

| Variable | Description |
|----------|-------------|
| `Qt6_DIR` | Path to Qt6 installation (Windows) |
| `QMAKE` | Path to qmake6 (Linux) |
| `SOURCE_DATE_EPOCH` | Unix timestamp for reproducible builds |

## CI/CD Integration

The packaging scripts are designed to work in CI environments:

```yaml
# Example GitHub Actions snippet
- name: Package (Windows)
  if: runner.os == 'Windows'
  run: |
    cmake --preset rel
    cmake --build --preset rel
    powershell -File packaging/package-windows.ps1

- name: Package (Linux)
  if: runner.os == 'Linux'
  run: |
    cmake --preset rel
    cmake --build --preset rel
    ./packaging/package-linux.sh
```

See `.github/workflows/` for complete CI configuration (when available).
