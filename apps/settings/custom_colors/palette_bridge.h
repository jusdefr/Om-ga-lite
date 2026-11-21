#ifndef OMEGA_LITE_CUSTOM_COLORS_PALETTE_BRIDGE_H
#define OMEGA_LITE_CUSTOM_COLORS_PALETTE_BRIDGE_H

#include <kandinsky/color.h>

#include "color_rgb.h"

namespace OmegaLite {
namespace Colors {

/* Converts a stored 24-bit color into the RGB565 color the screen draws.
 * This is the only place where the settings colors touch kandinsky, which
 * keeps the rest of the module testable on a computer. */
inline KDColor toKDColor(RgbColor color) {
  return KDColor::RGB888(color.red(), color.green(), color.blue());
}

} // namespace Colors
} // namespace OmegaLite

#endif
