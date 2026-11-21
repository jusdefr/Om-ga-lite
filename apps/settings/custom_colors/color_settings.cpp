#include "color_settings.h"

#include <assert.h>
#include <string.h>

namespace OmegaLite {
namespace Colors {

ColorSettings::ColorSettings(ColorScheme * scheme, ColorStore * store) :
    m_scheme(scheme),
    m_store(store),
    m_editing(false),
    m_storedOverrideCount(0),
    m_lastLoadStatus(ColorStore::LoadStatus::Absent) {
  assert(m_scheme != nullptr);
  assert(m_store != nullptr);
  snapshot();
}

void ColorSettings::snapshot() {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_snapshot[i] = m_scheme->color(static_cast<Role>(i));
  }
}

bool ColorSettings::load() {
  uint8_t buffer[ColorOverrides::k_maxSerializedSize];
  size_t size = 0;
  const ColorStore::LoadStatus status = m_store->load(buffer, sizeof(buffer), &size);
  m_lastLoadStatus = status;

  if (status != ColorStore::LoadStatus::Loaded) {
    /* Absent or corrupted record: the theme defaults stay in place. */
    m_scheme->resetToDefaults();
    m_storedOverrideCount = 0;
    snapshot();
    return false;
  }

  ColorOverrides overrides;
  if (!overrides.deserialize(buffer, size)) {
    m_lastLoadStatus = ColorStore::LoadStatus::Corrupted;
    m_scheme->resetToDefaults();
    m_storedOverrideCount = 0;
    snapshot();
    return false;
  }

  m_scheme->resetToDefaults();
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    const Role role = static_cast<Role>(i);
    if (overrides.isSet(role)) {
      m_scheme->setColor(role, overrides.colorOr(role, m_scheme->defaultColor(role)));
    }
  }
  m_storedOverrideCount = overrides.numberOfOverrides();
  snapshot();
  return true;
}

bool ColorSettings::save() {
  ColorOverrides overrides;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    const Role role = static_cast<Role>(i);
    if (!m_scheme->isDefault(role)) {
      overrides.set(role, m_scheme->color(role));
    }
  }

  bool success = true;
  if (overrides.numberOfOverrides() == 0) {
    /* No override left: erase the record instead of storing an empty blob, so
     * that a future default palette change applies immediately. */
    success = m_store->clear();
  } else {
    uint8_t buffer[ColorOverrides::k_maxSerializedSize];
    const size_t size = overrides.serialize(buffer, sizeof(buffer));
    success = size != 0 && m_store->save(buffer, size);
  }

  if (success) {
    m_storedOverrideCount = overrides.numberOfOverrides();
    m_lastLoadStatus = overrides.numberOfOverrides() == 0
                           ? ColorStore::LoadStatus::Absent
                           : ColorStore::LoadStatus::Loaded;
    m_editing = false;
    snapshot();
  }
  return success;
}

bool ColorSettings::resetToDefaults() {
  m_scheme->resetToDefaults();
  const bool success = m_store->clear();
  if (success) {
    m_storedOverrideCount = 0;
    m_lastLoadStatus = ColorStore::LoadStatus::Absent;
    snapshot();
  }
  return success;
}

void ColorSettings::resetToThemeDefaults() {
  /* Only the draft changes: Save will erase the record (no override left),
   * Cancel will bring the previous colors back from the snapshot. */
  m_scheme->resetToDefaults();
}

void ColorSettings::beginEdit() {
  if (!m_editing) {
    snapshot();
    m_editing = true;
  }
}

void ColorSettings::revertEdit() {
  if (!m_editing) {
    return;
  }
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_scheme->setColor(static_cast<Role>(i), m_snapshot[i]);
  }
  m_editing = false;
}

bool ColorSettings::hasUnsavedChanges() const {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    if (m_snapshot[i] != m_scheme->color(static_cast<Role>(i))) {
      return true;
    }
  }
  return false;
}

} // namespace Colors
} // namespace OmegaLite
