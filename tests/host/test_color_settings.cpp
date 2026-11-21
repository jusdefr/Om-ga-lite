#include "test_framework.h"

#include "color_settings.h"

using namespace OmegaLite::Colors;

namespace {

const RgbColor k_testDefaults[k_numberOfRoles] = {
  RgbColor(0x0B, 0x12, 0x20), RgbColor(0x11, 0x1A, 0x2B), RgbColor(0x1B, 0x27, 0x40),
  RgbColor(0xEE, 0xF4, 0xFF), RgbColor(0x9F, 0xB0, 0xCC), RgbColor(0x2F, 0xD3, 0xC4),
  RgbColor(0x2F, 0xD3, 0xC4), RgbColor(0x11, 0x1A, 0x2B), RgbColor(0x1B, 0x27, 0x40),
  RgbColor(0x25, 0x40, 0x5F), RgbColor(0x25, 0x40, 0x5F), RgbColor(0x26, 0x35, 0x4F),
  RgbColor(0x2F, 0xD3, 0xC4), RgbColor(0x2F, 0xD3, 0xC4), RgbColor(0x2A, 0x3B, 0x57),
  RgbColor(0x1A, 0x27, 0x40),
};

struct Fixture {
  ColorScheme scheme;
  MemoryColorStore store;
  ColorSettings settings;
  Fixture() : settings(&scheme, &store) { scheme.setDefaults(k_testDefaults); }
};

} // namespace

TEST(SchemeStartsAtDefaults) {
  Fixture fixture;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    const Role role = static_cast<Role>(i);
    CHECK(fixture.scheme.color(role) == k_testDefaults[i]);
    CHECK(fixture.scheme.isDefault(role));
  }
  CHECK_EQ(fixture.scheme.overrideMask(), 0u);
  CHECK(!fixture.scheme.hasOverrides());
}

TEST(SettingAColorUpdatesTheOverrideMask) {
  Fixture fixture;
  fixture.scheme.setColor(Role::AccentText, RgbColor(1, 2, 3));
  CHECK(!fixture.scheme.isDefault(Role::AccentText));
  CHECK_EQ(fixture.scheme.overrideMask(), 1u << roleIndex(Role::AccentText));
  fixture.scheme.setColor(Role::AccentText, k_testDefaults[roleIndex(Role::AccentText)]);
  CHECK_EQ(fixture.scheme.overrideMask(), 0u);
}

TEST(SavePersistsOnlyOverridesAndLoadRestoresThem) {
  Fixture fixture;
  fixture.scheme.setColor(Role::Background, RgbColor(0x01, 0x02, 0x03));
  fixture.scheme.setColor(Role::Toolbar, RgbColor(0xAA, 0xBB, 0xCC));
  CHECK(fixture.settings.save());
  CHECK(fixture.store.isPresent());

  ColorScheme reloadedScheme;
  reloadedScheme.setDefaults(k_testDefaults);
  MemoryColorStore sameStore = fixture.store;
  ColorSettings reloadedSettings(&reloadedScheme, &sameStore);
  CHECK(reloadedSettings.load());
  CHECK(reloadedSettings.storedOverrideCount() == 2);
  CHECK(reloadedScheme.color(Role::Background) == RgbColor(0x01, 0x02, 0x03));
  CHECK(reloadedScheme.color(Role::Toolbar) == RgbColor(0xAA, 0xBB, 0xCC));
  CHECK(reloadedScheme.color(Role::PrimaryText) == k_testDefaults[roleIndex(Role::PrimaryText)]);
}

TEST(SavingWithoutOverridesErasesTheRecord) {
  Fixture fixture;
  fixture.scheme.setColor(Role::Border, RgbColor(9, 9, 9));
  CHECK(fixture.settings.save());
  CHECK(fixture.store.isPresent());
  fixture.scheme.resetToDefaults();
  CHECK(fixture.settings.save());
  CHECK(!fixture.store.isPresent());
}

TEST(ResetToDefaultsRestoresThePaletteAndClearsStorage) {
  Fixture fixture;
  fixture.scheme.setColor(Role::GraphAxis, RgbColor(1, 1, 1));
  CHECK(fixture.settings.save());
  CHECK(fixture.settings.resetToDefaults());
  CHECK(!fixture.store.isPresent());
  CHECK(fixture.scheme.color(Role::GraphAxis) == k_testDefaults[roleIndex(Role::GraphAxis)]);
  CHECK_EQ(fixture.settings.storedOverrideCount(), 0);
}

TEST(CorruptedStoreFallsBackToDefaults) {
  Fixture fixture;
  fixture.scheme.setColor(Role::Selection, RgbColor(5, 6, 7));
  CHECK(fixture.settings.save());
  fixture.store.corrupt();
  CHECK(!fixture.settings.load());
  CHECK(fixture.settings.lastLoadStatus() == ColorStore::LoadStatus::Corrupted);
  CHECK(fixture.scheme.color(Role::Selection) == k_testDefaults[roleIndex(Role::Selection)]);
  CHECK_EQ(fixture.settings.storedOverrideCount(), 0);
}

TEST(AbsentStoreLeavesDefaultsInPlace) {
  Fixture fixture;
  CHECK(!fixture.settings.load());
  CHECK(fixture.settings.lastLoadStatus() == ColorStore::LoadStatus::Absent);
  CHECK(fixture.scheme.color(Role::Background) == k_testDefaults[0]);
}

TEST(EditingCanBeReverted) {
  Fixture fixture;
  fixture.settings.beginEdit();
  CHECK(fixture.settings.isEditing());
  fixture.scheme.setColor(Role::PrimaryText, RgbColor(0x12, 0x34, 0x56));
  CHECK(fixture.settings.hasUnsavedChanges());
  fixture.settings.revertEdit();
  CHECK(!fixture.settings.isEditing());
  CHECK(!fixture.settings.hasUnsavedChanges());
  CHECK(fixture.scheme.color(Role::PrimaryText) == k_testDefaults[roleIndex(Role::PrimaryText)]);
}

TEST(SavingAnEditEndsTheEditSession) {
  Fixture fixture;
  fixture.settings.beginEdit();
  fixture.scheme.setColor(Role::SecondaryText, RgbColor(0x42, 0x42, 0x42));
  CHECK(fixture.settings.save());
  CHECK(!fixture.settings.isEditing());
  CHECK(!fixture.settings.hasUnsavedChanges());
  CHECK(fixture.settings.storedOverrideCount() == 1);
}

TEST(RevertWithoutEditDoesNothing) {
  Fixture fixture;
  fixture.scheme.setColor(Role::Control, RgbColor(7, 7, 7));
  fixture.settings.revertEdit();
  CHECK(fixture.scheme.color(Role::Control) == RgbColor(7, 7, 7));
}

TEST(RestoreDuringEditKeepsStorageUntilSaveOrCancel) {
  Fixture fixture;
  /* A saved, non default palette. */
  fixture.scheme.setColor(Role::AccentText, RgbColor(0x12, 0x34, 0x56));
  CHECK(fixture.settings.save());
  const size_t savedSize = fixture.store.size();
  CHECK(savedSize > 0);

  /* Restore is only a draft change: the record must stay untouched. */
  fixture.settings.beginEdit();
  fixture.settings.resetToThemeDefaults();
  CHECK(fixture.scheme.color(Role::AccentText) == k_testDefaults[roleIndex(Role::AccentText)]);
  CHECK(fixture.store.isPresent());
  CHECK(fixture.store.size() == savedSize);
  CHECK(fixture.settings.hasUnsavedChanges());

  /* Cancel brings the saved colors back, storage included. */
  fixture.settings.revertEdit();
  CHECK(fixture.scheme.color(Role::AccentText) == RgbColor(0x12, 0x34, 0x56));
  CHECK(fixture.store.isPresent());
  CHECK(fixture.store.size() == savedSize);
  CHECK(fixture.settings.storedOverrideCount() == 1);
}

TEST(RestoreDuringEditThenSavePersistsTheDefaults) {
  Fixture fixture;
  fixture.scheme.setColor(Role::GraphCurve, RgbColor(9, 8, 7));
  CHECK(fixture.settings.save());
  CHECK(fixture.store.isPresent());

  fixture.settings.beginEdit();
  fixture.settings.resetToThemeDefaults();
  CHECK(fixture.settings.save());
  CHECK(!fixture.store.isPresent());
  CHECK_EQ(fixture.settings.storedOverrideCount(), 0);
  CHECK(fixture.scheme.color(Role::GraphCurve) == k_testDefaults[roleIndex(Role::GraphCurve)]);
}

TEST(PersistedBlobIsSmallEnoughForTheRecord) {
  Fixture fixture;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    fixture.scheme.setColor(static_cast<Role>(i), RgbColor(i, i, i));
  }
  CHECK(fixture.settings.save());
  CHECK(fixture.store.size() == ColorOverrides::k_maxSerializedSize);
  CHECK(fixture.store.size() <= 76);
}

TEST(LoadAppliesPartialOverridesOnTopOfDefaults) {
  Fixture fixture;
  fixture.scheme.setColor(Role::GraphGrid, RgbColor(0x10, 0x20, 0x30));
  CHECK(fixture.settings.save());

  ColorScheme scheme;
  scheme.setDefaults(k_testDefaults);
  MemoryColorStore store = fixture.store;
  ColorSettings settings(&scheme, &store);
  CHECK(settings.load());
  CHECK(scheme.color(Role::GraphGrid) == RgbColor(0x10, 0x20, 0x30));
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    if (i == roleIndex(Role::GraphGrid)) {
      continue;
    }
    CHECK(scheme.color(static_cast<Role>(i)) == k_testDefaults[i]);
  }
}
