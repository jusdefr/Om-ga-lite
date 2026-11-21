#include "color_overrides.h"

#include <string.h>

namespace OmegaLite {
namespace Colors {

namespace {

constexpr char k_magic[4] = {'O', 'L', 'C', '1'};

int numberOfBitsSet(uint32_t value) {
  int count = 0;
  while (value != 0) {
    count += value & 1u;
    value >>= 1;
  }
  return count;
}

} // namespace

uint32_t ColorOverrides::checksumOf(const uint8_t * buffer, size_t size) {
  /* FNV-1a, 32 bit. Chosen because it is a few lines of code and needs no
   * table, which matters in a firmware. */
  uint32_t hash = 2166136261u;
  for (size_t i = 0; i < size; i++) {
    hash ^= buffer[i];
    hash *= 16777619u;
  }
  return hash;
}

ColorOverrides::ColorOverrides() : m_setMask(0) {
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_colors[i] = RgbColor();
  }
}

void ColorOverrides::clear() {
  m_setMask = 0;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_colors[i] = RgbColor();
  }
}

void ColorOverrides::set(Role role, RgbColor color) {
  if (!isValidRole(roleIndex(role))) {
    return;
  }
  m_colors[roleIndex(role)] = color;
  m_setMask |= (1u << roleIndex(role));
}

void ColorOverrides::unset(Role role) {
  if (!isValidRole(roleIndex(role))) {
    return;
  }
  m_colors[roleIndex(role)] = RgbColor();
  m_setMask &= ~(1u << roleIndex(role));
}

bool ColorOverrides::isSet(Role role) const {
  if (!isValidRole(roleIndex(role))) {
    return false;
  }
  return (m_setMask & (1u << roleIndex(role))) != 0;
}

RgbColor ColorOverrides::colorOr(Role role, RgbColor fallback) const {
  return isSet(role) ? m_colors[roleIndex(role)] : fallback;
}

uint8_t ColorOverrides::numberOfOverrides() const {
  return static_cast<uint8_t>(numberOfBitsSet(m_setMask));
}

size_t ColorOverrides::serialize(uint8_t * buffer, size_t capacity) const {
  if (buffer == nullptr) {
    return 0;
  }
  const uint8_t count = numberOfOverrides();
  const size_t size = k_headerSize + count * k_entrySize + k_checksumSize;
  if (capacity < size) {
    return 0;
  }
  memcpy(buffer, k_magic, sizeof(k_magic));
  buffer[4] = k_formatVersion;
  buffer[5] = count;
  buffer[6] = 0;
  buffer[7] = 0;
  size_t offset = k_headerSize;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    if ((m_setMask & (1u << i)) == 0) {
      continue;
    }
    buffer[offset++] = i;
    buffer[offset++] = m_colors[i].red();
    buffer[offset++] = m_colors[i].green();
    buffer[offset++] = m_colors[i].blue();
  }
  const uint32_t checksum = checksumOf(buffer, offset);
  buffer[offset++] = static_cast<uint8_t>(checksum & 0xFF);
  buffer[offset++] = static_cast<uint8_t>((checksum >> 8) & 0xFF);
  buffer[offset++] = static_cast<uint8_t>((checksum >> 16) & 0xFF);
  buffer[offset++] = static_cast<uint8_t>((checksum >> 24) & 0xFF);
  return offset;
}

bool ColorOverrides::deserialize(const uint8_t * buffer, size_t size) {
  if (buffer == nullptr || size < k_headerSize + k_checksumSize) {
    return false;
  }
  if (memcmp(buffer, k_magic, sizeof(k_magic)) != 0) {
    return false;
  }
  if (buffer[4] != k_formatVersion) {
    return false;
  }
  const uint8_t count = buffer[5];
  if (count > k_numberOfRoles) {
    return false;
  }
  const size_t expectedSize = k_headerSize + count * k_entrySize + k_checksumSize;
  if (size != expectedSize) {
    return false;
  }
  const size_t checksumOffset = size - k_checksumSize;
  const uint32_t expectedChecksum =
      static_cast<uint32_t>(buffer[checksumOffset]) |
      (static_cast<uint32_t>(buffer[checksumOffset + 1]) << 8) |
      (static_cast<uint32_t>(buffer[checksumOffset + 2]) << 16) |
      (static_cast<uint32_t>(buffer[checksumOffset + 3]) << 24);
  if (checksumOf(buffer, checksumOffset) != expectedChecksum) {
    return false;
  }
  uint32_t mask = 0;
  RgbColor colors[k_numberOfRoles];
  int previousRole = -1;
  for (uint8_t entry = 0; entry < count; entry++) {
    const size_t offset = k_headerSize + entry * k_entrySize;
    const uint8_t role = buffer[offset];
    if (!isValidRole(role) || static_cast<int>(role) <= previousRole) {
      return false;
    }
    previousRole = role;
    mask |= (1u << role);
    colors[role] = RgbColor(buffer[offset + 1], buffer[offset + 2], buffer[offset + 3]);
  }
  m_setMask = mask;
  for (uint8_t i = 0; i < k_numberOfRoles; i++) {
    m_colors[i] = (mask & (1u << i)) != 0 ? colors[i] : RgbColor();
  }
  return true;
}

} // namespace Colors
} // namespace OmegaLite
