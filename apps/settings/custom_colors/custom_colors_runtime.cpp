#include "custom_colors_runtime.h"

#include "ion_color_store.h"
#include "palette_defaults.h"

namespace OmegaLite {
namespace Colors {

ColorScheme & scheme() {
  static ColorScheme s_scheme;
  static bool s_initialized = false;
  if (!s_initialized) {
    applyThemeDefaults(&s_scheme);
    s_initialized = true;
  }
  return s_scheme;
}

ColorSettings & settings() {
  static IonColorStore s_store;
  static ColorSettings s_settings(&scheme(), &s_store);
  return s_settings;
}

bool loadAtStartup() { return settings().load(); }

} // namespace Colors
} // namespace OmegaLite
