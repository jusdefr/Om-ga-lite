#ifndef OMEGA_LITE_CUSTOM_COLORS_COLOR_ROLES_H
#define OMEGA_LITE_CUSTOM_COLORS_COLOR_ROLES_H

#include <stdint.h>

namespace OmegaLite {
namespace Colors {

/* The color roles exposed in the "Customize colors" settings page.
 *
 * Every role is mapped onto one key of the Omega theme format (see
 * paletteKey()). Keeping the list small is deliberate: these are the colors a
 * student can reason about, while the Omega palette itself has more than a
 * hundred keys that stay derived from the theme file and are not editable. */
enum class Role : uint8_t {
  Background = 0,
  AppBackground,
  SecondaryBackground,
  PrimaryText,
  SecondaryText,
  AccentText,
  Toolbar,
  MenuBackground,
  Button,
  ButtonSelected,
  Selection,
  Border,
  Control,
  GraphCurve,
  GraphGrid,
  GraphAxis,
  NumberOfRoles
};

constexpr uint8_t k_numberOfRoles = static_cast<uint8_t>(Role::NumberOfRoles);

constexpr uint8_t roleIndex(Role role) { return static_cast<uint8_t>(role); }

bool isValidRole(uint8_t index);

/* Stable identifier, used by the settings record and by tests. Never
 * translated, never reordered. */
const char * roleIdentifier(Role role);

/* Short label shown in the customization list. */
const char * roleLabel(Role role);

/* Key of the color in an Omega theme file, using a dot for nested groups:
 * "Background.Hard", "Button.BackgroundSelected", "Grid.PrimaryLine"...
 * The placeholder group value is written without a suffix ("Toolbar"). */
const char * paletteKey(Role role);

} // namespace Colors
} // namespace OmegaLite

#endif
