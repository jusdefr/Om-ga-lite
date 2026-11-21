#ifndef OMEGA_LITE_CUSTOM_COLORS_COLOR_SETTINGS_H
#define OMEGA_LITE_CUSTOM_COLORS_COLOR_SETTINGS_H

#include <stdint.h>

#include "color_overrides.h"
#include "color_scheme.h"
#include "color_store.h"

namespace OmegaLite {
namespace Colors {

/* Ties a ColorScheme to its persistence backend.
 *
 * Responsibilities:
 *  - load(): read the record, apply the saved overrides, fall back to the
 *    theme defaults when the record is absent or corrupted;
 *  - save(): write only the roles that differ from the defaults;
 *  - resetToDefaults(): restore the theme palette and erase the record;
 *  - beginEdit()/revertEdit(): support "cancel before saving". */
class ColorSettings {
public:
  ColorSettings(ColorScheme * scheme, ColorStore * store);

  /* Returns true when saved overrides were applied. Never leaves the scheme in
   * a partially updated state. */
  bool load();
  /* Returns false if the record could not be written. */
  bool save();
  /* Restores the theme palette and erases the record immediately. */
  bool resetToDefaults();
  /* Restores the theme palette for the current edit only: storage is left
   * untouched, so "Restore" followed by "Cancel" keeps the saved colors. */
  void resetToThemeDefaults();

  void beginEdit();
  void revertEdit();
  bool isEditing() const { return m_editing; }
  bool hasUnsavedChanges() const;

  ColorScheme * scheme() const { return m_scheme; }

  /* Number of overridden roles currently stored (0 when nothing is saved). */
  uint8_t storedOverrideCount() const { return m_storedOverrideCount; }
  ColorStore::LoadStatus lastLoadStatus() const { return m_lastLoadStatus; }

private:
  uint32_t currentMask() const { return m_scheme->overrideMask(); }
  void snapshot();

  ColorScheme * m_scheme;
  ColorStore * m_store;
  RgbColor m_snapshot[k_numberOfRoles];
  bool m_editing;
  uint8_t m_storedOverrideCount;
  ColorStore::LoadStatus m_lastLoadStatus;
};

} // namespace Colors
} // namespace OmegaLite

#endif
