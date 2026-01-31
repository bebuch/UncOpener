#ifndef UNCOPENER_APPICON_HPP
#define UNCOPENER_APPICON_HPP

#include <QIcon>

/// Load the application icon from SVG resource with pre-rendered pixmaps
/// This ensures proper display on all platforms including Wayland
[[nodiscard]] QIcon loadAppIcon();

#endif // UNCOPENER_APPICON_HPP
