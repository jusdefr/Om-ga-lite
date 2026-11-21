#ifndef OMEGA_LITE_CUSTOM_COLORS_COLOR_SCHEME_H
#define OMEGA_LITE_CUSTOM_COLORS_COLOR_SCHEME_H

#include "color_roles.h"
#include "color_rgb.h"

namespace OmegaLite {
namespace Colors {

/* Holds one color per role, plus the defaults it started from.
 *
 * The scheme is the single source of truth while the customization page is
 * open: editing a role only touches the scheme, which lets the live preview
 * render the pending colors and the user cancel before saving. */
class ColorScheme {
public:
  ColorScheme();

  /* Copies the defaults and resets every role to them. */
  void setDefaults(const RgbColor * defaults);
  const RgbColor & color(Role role) const;
  const RgbColor & defaultColor(Role role) const;
  void setColor(Role role, RgbColor color);
  void resetToDefaults();
  bool isDefault(Role role) const;
  /* Bit i is set when role i differs from its default. */
  uint32_t overrideMask() const;
  bool hasOverrides() const { return overrideMask() != 0; }

private:
  RgbColor m_colors[k_numberOfRoles];
  RgbColor m_defaults[k_numberOfRoles];
};

} // namespace Colors
} // namespace OmegaLite

#endif
