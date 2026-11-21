#ifndef OMEGA_LITE_CUSTOM_COLORS_RUNTIME_H
#define OMEGA_LITE_CUSTOM_COLORS_RUNTIME_H

#include "color_settings.h"
#include "color_scheme.h"

namespace OmegaLite {
namespace Colors {

/* Process wide palette used by the screens that support customization.
 *
 * Initialization order is handled lazily: scheme() installs the theme
 * defaults the first time it is called, and settings() then ties that scheme
 * to the Ion storage record. */
ColorScheme & scheme();
ColorSettings & settings();

/* Reads the saved overrides and applies them to the runtime palette.
 * Call once at boot, before the first window is drawn. Returns true when
 * saved colors were applied; on a missing or corrupted record the theme
 * defaults stay in place and the function returns false. */
bool loadAtStartup();

} // namespace Colors
} // namespace OmegaLite

#endif
