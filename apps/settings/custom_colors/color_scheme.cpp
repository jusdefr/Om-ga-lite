#include "color_scheme.h"

#include <assert.h>

namespace OmegaLite {
namespace Colors {

ColorScheme::ColorScheme() {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_colors[i] = RgbColor();
    m_defaults[i] = RgbColor();
  }
}

void ColorScheme::setDefaults(const RgbColor * defaults) {
  assert(defaults != nullptr);
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_defaults[i] = defaults[i];
    m_colors[i] = defaults[i];
  }
}

const RgbColor & ColorScheme::color(Role role) const {
  assert(isValidRole(roleIndex(role)));
  return m_colors[roleIndex(role)];
}

const RgbColor & ColorScheme::defaultColor(Role role) const {
  assert(isValidRole(roleIndex(role)));
  return m_defaults[roleIndex(role)];
}

void ColorScheme::setColor(Role role, RgbColor color) {
  assert(isValidRole(roleIndex(role)));
  m_colors[roleIndex(role)] = color;
}

void ColorScheme::resetToDefaults() {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_colors[i] = m_defaults[i];
  }
}

bool ColorScheme::isDefault(Role role) const {
  return color(role) == defaultColor(role);
}

uint32_t ColorScheme::overrideMask() const {
  uint32_t mask = 0;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    if (m_colors[i] != m_defaults[i]) {
      mask |= (1u << i);
    }
  }
  return mask;
}

} // namespace Colors
} // namespace OmegaLite
