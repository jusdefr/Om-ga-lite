#include <cstring>

#include "test_framework.h"

#include "color_overrides.h"

using namespace OmegaLite::Colors;

namespace {

/* Builds a blob by hand so that invalid combinations can be tested. */
size_t craftBlob(uint8_t * buffer, uint8_t count, const uint8_t * entries) {
  std::memcpy(buffer, "OLC1", 4);
  buffer[4] = 1;
  buffer[5] = count;
  buffer[6] = 0;
  buffer[7] = 0;
  if (count != 0) {
    std::memcpy(buffer + ColorOverrides::k_headerSize, entries, count * 4u);
  }
  const size_t size = ColorOverrides::k_headerSize + count * 4u;
  const uint32_t checksum = ColorOverrides::checksumOf(buffer, size);
  buffer[size] = static_cast<uint8_t>(checksum & 0xFF);
  buffer[size + 1] = static_cast<uint8_t>((checksum >> 8) & 0xFF);
  buffer[size + 2] = static_cast<uint8_t>((checksum >> 16) & 0xFF);
  buffer[size + 3] = static_cast<uint8_t>((checksum >> 24) & 0xFF);
  return size + 4;
}

} // namespace

TEST(EmptyOverridesSerializeToHeaderAndChecksum) {
  ColorOverrides overrides;
  uint8_t buffer[ColorOverrides::k_maxSerializedSize];
  const size_t size = overrides.serialize(buffer, sizeof(buffer));
  CHECK_EQ(size, ColorOverrides::k_headerSize + ColorOverrides::k_checksumSize);
  CHECK_EQ(overrides.numberOfOverrides(), 0);
}

TEST(OverridesRoundTrip) {
  ColorOverrides overrides;
  overrides.set(Role::Background, RgbColor(11, 18, 32));
  overrides.set(Role::GraphAxis, RgbColor(26, 39, 64));
  CHECK_EQ(overrides.numberOfOverrides(), 2);
  CHECK(overrides.isSet(Role::Background));
  CHECK(!overrides.isSet(Role::PrimaryText));

  uint8_t buffer[ColorOverrides::k_maxSerializedSize];
  const size_t size = overrides.serialize(buffer, sizeof(buffer));
  CHECK_EQ(size, 8u + 2u * 4u + 4u);

  ColorOverrides reloaded;
  CHECK(reloaded.deserialize(buffer, size));
  CHECK_EQ(reloaded.numberOfOverrides(), 2);
  CHECK(reloaded.colorOr(Role::Background, RgbColor()) == RgbColor(11, 18, 32));
  CHECK(reloaded.colorOr(Role::GraphAxis, RgbColor()) == RgbColor(26, 39, 64));
  CHECK(reloaded.colorOr(Role::PrimaryText, RgbColor(1, 2, 3)) == RgbColor(1, 2, 3));
}

TEST(UnsetRemovesAnOverride) {
  ColorOverrides overrides;
  overrides.set(Role::Toolbar, RgbColor(47, 211, 196));
  overrides.unset(Role::Toolbar);
  CHECK_EQ(overrides.numberOfOverrides(), 0);
  CHECK(!overrides.isSet(Role::Toolbar));
}

TEST(SerializeFailsWhenBufferIsTooSmall) {
  ColorOverrides overrides;
  overrides.set(Role::Background, RgbColor(1, 2, 3));
  uint8_t buffer[ColorOverrides::k_maxSerializedSize];
  CHECK_EQ(overrides.serialize(buffer, 4), 0);
  CHECK_EQ(overrides.serialize(nullptr, sizeof(buffer)), 0);
}

TEST(DeserializeRejectsBadMagic) {
  uint8_t buffer[64];
  const size_t size = craftBlob(buffer, 0, nullptr);
  buffer[0] = 'X';
  const size_t fixed = craftBlob(buffer, 0, nullptr);
  buffer[0] = 'X';
  const uint32_t checksum = ColorOverrides::checksumOf(buffer, fixed - 4);
  buffer[fixed - 4] = static_cast<uint8_t>(checksum & 0xFF);
  buffer[fixed - 3] = static_cast<uint8_t>((checksum >> 8) & 0xFF);
  buffer[fixed - 2] = static_cast<uint8_t>((checksum >> 16) & 0xFF);
  buffer[fixed - 1] = static_cast<uint8_t>((checksum >> 24) & 0xFF);
  ColorOverrides overrides;
  CHECK(!overrides.deserialize(buffer, size));
}

TEST(DeserializeRejectsUnsupportedVersion) {
  uint8_t buffer[64];
  const size_t size = craftBlob(buffer, 0, nullptr);
  buffer[4] = 2;
  const uint32_t checksum = ColorOverrides::checksumOf(buffer, size - 4);
  buffer[size - 4] = static_cast<uint8_t>(checksum & 0xFF);
  buffer[size - 3] = static_cast<uint8_t>((checksum >> 8) & 0xFF);
  buffer[size - 2] = static_cast<uint8_t>((checksum >> 16) & 0xFF);
  buffer[size - 1] = static_cast<uint8_t>((checksum >> 24) & 0xFF);
  ColorOverrides overrides;
  CHECK(!overrides.deserialize(buffer, size));
}

TEST(DeserializeRejectsTruncatedBlob) {
  ColorOverrides overrides;
  overrides.set(Role::Background, RgbColor(1, 2, 3));
  uint8_t buffer[ColorOverrides::k_maxSerializedSize];
  const size_t size = overrides.serialize(buffer, sizeof(buffer));
  ColorOverrides reloaded;
  CHECK(!reloaded.deserialize(buffer, size - 1));
  CHECK(!reloaded.deserialize(buffer, size + 1));
  CHECK(!reloaded.deserialize(buffer, 0));
  CHECK(!reloaded.deserialize(nullptr, size));
}

TEST(DeserializeRejectsBadChecksum) {
  ColorOverrides overrides;
  overrides.set(Role::Background, RgbColor(1, 2, 3));
  uint8_t buffer[ColorOverrides::k_maxSerializedSize];
  const size_t size = overrides.serialize(buffer, sizeof(buffer));
  buffer[ColorOverrides::k_headerSize + 1] ^= 0xFF;
  ColorOverrides reloaded;
  CHECK(!reloaded.deserialize(buffer, size));
}

TEST(DeserializeRejectsTooManyEntries) {
  uint8_t buffer[ColorOverrides::k_maxSerializedSize + 8];
  uint8_t entries[17 * 4];
  for (int i = 0; i < 17; i++) {
    entries[i * 4] = static_cast<uint8_t>(i);
  }
  const size_t size = craftBlob(buffer, 17, entries);
  ColorOverrides overrides;
  CHECK(!overrides.deserialize(buffer, size));
}

TEST(DeserializeRejectsInvalidRoleIndex) {
  uint8_t buffer[64];
  uint8_t entries[4] = {42, 1, 2, 3};
  const size_t size = craftBlob(buffer, 1, entries);
  ColorOverrides overrides;
  CHECK(!overrides.deserialize(buffer, size));
}

TEST(DeserializeRejectsUnsortedRoles) {
  uint8_t buffer[64];
  uint8_t entries[8] = {5, 1, 2, 3, 2, 4, 5, 6};
  const size_t size = craftBlob(buffer, 2, entries);
  ColorOverrides overrides;
  CHECK(!overrides.deserialize(buffer, size));
}

TEST(FailedDeserializeLeavesObjectUntouched) {
  ColorOverrides overrides;
  overrides.set(Role::Border, RgbColor(1, 2, 3));
  uint8_t buffer[64];
  uint8_t entries[4] = {0, 9, 9, 9};
  const size_t size = craftBlob(buffer, 1, entries);
  buffer[size - 1] ^= 0xFF;
  CHECK(!overrides.deserialize(buffer, size));
  CHECK(overrides.isSet(Role::Border));
  CHECK(overrides.colorOr(Role::Border, RgbColor()) == RgbColor(1, 2, 3));
}
