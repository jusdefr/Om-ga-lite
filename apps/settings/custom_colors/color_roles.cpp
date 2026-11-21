#include "color_roles.h"

#include <stddef.h>

namespace OmegaLite {
namespace Colors {

namespace {

struct RoleDefinition {
  const char * identifier;
  const char * label;
  const char * paletteKey;
};

/* The order must match the Role enum. */
constexpr RoleDefinition k_roleDefinitions[k_numberOfRoles] = {
  {"Background", "Fond general", "Background.Hard"},
  {"AppBackground", "Fond des applications", "Background.Apps"},
  {"SecondaryBackground", "Fond secondaire", "Background.AppsSecondary"},
  {"PrimaryText", "Texte principal", "PrimaryText"},
  {"SecondaryText", "Texte secondaire", "SecondaryText"},
  {"AccentText", "Texte accentue", "AccentText"},
  {"Toolbar", "Barre d'outils", "Toolbar"},
  {"MenuBackground", "Menus", "SubMenu.Background"},
  {"Button", "Boutons", "Button.Background"},
  {"ButtonSelected", "Boutons selectionnes", "Button.BackgroundSelected"},
  {"Selection", "Couleur de selection", "List.CellBackgroundSelected"},
  {"Border", "Bordures", "List.CellBorder"},
  {"Control", "Controles", "Control.Enabled"},
  {"GraphCurve", "Courbes", "Probability.Curve"},
  {"GraphGrid", "Grille des graphiques", "Grid.PrimaryLine"},
  {"GraphAxis", "Axes des graphiques", "Grid.SecondaryLine"},
};

} // namespace

bool isValidRole(uint8_t index) { return index < k_numberOfRoles; }

const char * roleIdentifier(Role role) {
  const uint8_t index = roleIndex(role);
  if (!isValidRole(index)) {
    return "";
  }
  return k_roleDefinitions[index].identifier;
}

const char * roleLabel(Role role) {
  const uint8_t index = roleIndex(role);
  if (!isValidRole(index)) {
    return "";
  }
  return k_roleDefinitions[index].label;
}

const char * paletteKey(Role role) {
  const uint8_t index = roleIndex(role);
  if (!isValidRole(index)) {
    return "";
  }
  return k_roleDefinitions[index].paletteKey;
}

} // namespace Colors
} // namespace OmegaLite
