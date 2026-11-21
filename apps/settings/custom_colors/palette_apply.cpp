#include "palette_apply.h"

#include <escher/palette.h>

#include "palette_bridge.h"

namespace OmegaLite {
namespace Colors {

namespace {

/* One entry per role, in Role order. The mapping is deliberately explicit:
 * adding a role forces a compile error here instead of silently doing
 * nothing. */
void applyRole(Role role, const ColorScheme & scheme) {
  const KDColor color = toKDColor(scheme.color(role));
  switch (role) {
    case Role::Background:
      Palette::BackgroundHard = color;
      break;
    case Role::AppBackground:
      Palette::BackgroundApps = color;
      break;
    case Role::SecondaryBackground:
      Palette::BackgroundAppsSecondary = color;
      break;
    case Role::PrimaryText:
      Palette::PrimaryText = color;
      break;
    case Role::SecondaryText:
      Palette::SecondaryText = color;
      break;
    case Role::AccentText:
      Palette::AccentText = color;
      break;
    case Role::Toolbar:
      Palette::Toolbar = color;
      break;
    case Role::MenuBackground:
      Palette::SubMenuBackground = color;
      break;
    case Role::Button:
      Palette::ButtonBackground = color;
      break;
    case Role::ButtonSelected:
      Palette::ButtonBackgroundSelected = color;
      break;
    case Role::Selection:
      Palette::ListCellBackgroundSelected = color;
      break;
    case Role::Border:
      Palette::ListCellBorder = color;
      break;
    case Role::Control:
      Palette::ControlEnabled = color;
      break;
    case Role::GraphCurve:
      Palette::ProbabilityCurve = color;
      break;
    case Role::GraphGrid:
      Palette::GridPrimaryLine = color;
      break;
    case Role::GraphAxis:
      Palette::GridSecondaryLine = color;
      break;
    case Role::NumberOfRoles:
      break;
  }
}

} // namespace

void applySchemeToPalette(const ColorScheme & scheme) {
  Palette::resetToThemeDefaults();
  for (uint8_t index = 0; index < k_numberOfRoles; index++) {
    applyRole(static_cast<Role>(index), scheme);
  }
}

} // namespace Colors
} // namespace OmegaLite
