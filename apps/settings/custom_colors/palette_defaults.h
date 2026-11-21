#ifndef OMEGA_LITE_CUSTOM_COLORS_PALETTE_DEFAULTS_H
#define OMEGA_LITE_CUSTOM_COLORS_PALETTE_DEFAULTS_H

#include "color_roles.h"
#include "color_rgb.h"
#include "color_scheme.h"
#include "palette_defaults_generated.h"

namespace OmegaLite {
namespace Colors {

/* Default colours of the customizable roles, taken from the theme the
 * firmware was built with (see scripts/generate_palette_defaults.py). */
inline const RgbColor * defaultPalette() { return k_themeDefaultColors; }

inline void applyThemeDefaults(ColorScheme * scheme) {
  scheme->setDefaults(k_themeDefaultColors);
}

} // namespace Colors
} // namespace OmegaLite

#endif
