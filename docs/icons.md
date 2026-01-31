# Icon Generation

## Overview

UncOpener uses a single SVG source (`icon.svg`) to generate platform-specific icon assets.

## Source File

The source icon is located at `icon.svg` in the project root. Any changes to the application icon should be made to this file.

## Generated Assets

Generated assets are stored in the `assets/` directory:

```
assets/
├── icon.svg          # Copy of source for in-app use
├── linux/
│   ├── icon-16.png
│   ├── icon-24.png
│   ├── icon-32.png
│   ├── icon-48.png
│   ├── icon-64.png
│   ├── icon-128.png
│   ├── icon-256.png
│   └── icon-512.png
└── windows/
    └── icon.ico      # Multi-size ICO (16-256px)
```

## Regenerating Icons

To regenerate icons after modifying `icon.svg`:

```bash
./tools/generate-icons.sh
```

### Prerequisites

The script requires at least one of these SVG converters:
- **inkscape** (recommended)
- **rsvg-convert** (from librsvg)

Additional tools:
- **ImageMagick** (`convert`) - Required for Windows ICO generation
- **optipng** - Optional, for PNG optimization

### Installation

**Ubuntu/Debian:**
```bash
sudo apt install inkscape imagemagick optipng
```

**Fedora:**
```bash
sudo dnf install inkscape ImageMagick optipng
```

**macOS (Homebrew):**
```bash
brew install inkscape imagemagick optipng
```

## In-App Usage

The application uses SVG directly where possible via Qt's SVG support. The PNG versions are generated for:
- Desktop environment integration (Linux .desktop files)
- AppImage icons
- System tray (if applicable)

The Windows ICO is used for:
- Executable icon
- Installer icons
- Windows registry URL handler icon
