#ifndef OMEGA_LITE_CUSTOM_COLORS_COLOR_STORE_H
#define OMEGA_LITE_CUSTOM_COLORS_COLOR_STORE_H

#include <stddef.h>
#include <stdint.h>

#include "color_overrides.h"

namespace OmegaLite {
namespace Colors {

/* Where the serialized overrides live.
 *
 * The interface exists so the persistence logic can be tested on a computer
 * with MemoryColorStore, while the calculator uses IonColorStore, which wraps
 * an Ion::Storage record. */
class ColorStore {
public:
  enum class LoadStatus {
    Loaded,
    Absent,
    /* The record exists but cannot be used (bad checksum, truncated record,
     * unknown format). The caller must fall back to the theme defaults. */
    Corrupted
  };
  virtual ~ColorStore() {}
  /* Copies at most capacity bytes into buffer and sets sizeRead. */
  virtual LoadStatus load(uint8_t * buffer, size_t capacity, size_t * sizeRead) = 0;
  virtual bool save(const uint8_t * buffer, size_t size) = 0;
  virtual bool clear() = 0;
};

/* Volatile store used by the host tests and as a safe fallback when the
 * storage is unavailable. */
class MemoryColorStore : public ColorStore {
public:
  MemoryColorStore();
  LoadStatus load(uint8_t * buffer, size_t capacity, size_t * sizeRead) override;
  bool save(const uint8_t * buffer, size_t size) override;
  bool clear() override;
  bool isPresent() const { return m_size != 0; }
  size_t size() const { return m_size; }
  /* Makes the next load report Corrupted, to test the fallback path. */
  void corrupt() { m_corrupted = true; }

private:
  uint8_t m_buffer[ColorOverrides::k_maxSerializedSize];
  size_t m_size;
  bool m_corrupted;
};

} // namespace Colors
} // namespace OmegaLite

#endif
