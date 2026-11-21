#ifndef OMEGA_LITE_CUSTOM_COLORS_COLOR_OVERRIDES_H
#define OMEGA_LITE_CUSTOM_COLORS_COLOR_OVERRIDES_H

#include <stddef.h>
#include <stdint.h>

#include "color_roles.h"
#include "color_rgb.h"

namespace OmegaLite {
namespace Colors {

/* The set of colors that differ from the theme defaults, and its binary
 * serialization into an Ion storage record.
 *
 * Serialized layout (all integers little endian, size in bytes):
 *
 *   0        magic "OLC1"
 *   4        format version (1)
 *   5        number of entries N (0..16)
 *   6..7     reserved, zero
 *   8        N entries of 4 bytes: role index, red, green, blue
 *   8 + 4N   FNV-1a 32 checksum of all preceding bytes
 *
 * Entries are sorted by role index. Storing only the overrides keeps the
 * record small (at most 76 bytes) and makes it possible to change the default
 * palette in a future release without invalidating saved settings. */
class ColorOverrides {
public:
  constexpr static size_t k_headerSize = 8;
  constexpr static size_t k_entrySize = 4;
  constexpr static size_t k_checksumSize = 4;
  constexpr static size_t k_maxSerializedSize =
      k_headerSize + k_numberOfRoles * k_entrySize + k_checksumSize;
  constexpr static uint8_t k_formatVersion = 1;

  ColorOverrides();

  void clear();
  void set(Role role, RgbColor color);
  void unset(Role role);
  bool isSet(Role role) const;
  RgbColor colorOr(Role role, RgbColor fallback) const;
  uint8_t numberOfOverrides() const;
  /* Returns the number of bytes written, or 0 when the buffer is too small. */
  size_t serialize(uint8_t * buffer, size_t capacity) const;
  /* Returns false, leaving the object untouched, when the blob is malformed:
   * wrong size, wrong magic, unsupported version, too many entries, invalid or
   * unsorted role index, or bad checksum. */
  bool deserialize(const uint8_t * buffer, size_t size);

  /* FNV-1a 32 over a byte range. Exposed for tests and for diagnostics. */
  static uint32_t checksumOf(const uint8_t * buffer, size_t size);

private:
  RgbColor m_colors[k_numberOfRoles];
  uint32_t m_setMask;
};

} // namespace Colors
} // namespace OmegaLite

#endif
