#ifndef OMEGA_LITE_CUSTOM_COLORS_PALETTE_APPLY_H
#define OMEGA_LITE_CUSTOM_COLORS_PALETTE_APPLY_H

#include "color_scheme.h"

namespace OmegaLite {
namespace Colors {

/* Copies the sixteen customizable roles into the global Omega Palette.
 *
 * This requires the runtime palette: the integration patch makes every
 * Palette member a mutable KDColor (see scripts/generate_runtime_palette.py),
 * so assigning here changes the color used by every existing screen without
 * touching a single call site.
 *
 * Call it after a successful load, save or cancel. The editor only changes the
 * draft, which is why the rest of the interface updates on commit. */
void applySchemeToPalette(const ColorScheme & scheme);

} // namespace Colors
} // namespace OmegaLite

#endif
