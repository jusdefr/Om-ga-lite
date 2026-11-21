#ifndef OMEGA_LITE_CUSTOM_COLORS_ION_COLOR_STORE_H
#define OMEGA_LITE_CUSTOM_COLORS_ION_COLOR_STORE_H

#include <stddef.h>
#include <stdint.h>

#include "color_store.h"

namespace OmegaLite {
namespace Colors {

/* Stores the overrides in an Ion storage record named "omegalite.clr".
 *
 * Ion storage is the same mechanism Omega already uses for equations,
 * functions, sequences and Python scripts: a flat, CRC addressed record area
 * of 32 KiB in the internal flash, managed by ``Ion::Storage``. There is no
 * key/value API and no file system, so the serialized blob described in
 * color_overrides.h is written as a single record.
 *
 * The record is tiny (12 to 76 bytes) compared with the 32 KiB storage, and
 * destroying and recreating it keeps the code independent from Ion's
 * same-size rewrite rule. */
class IonColorStore : public ColorStore {
public:
  constexpr static const char * k_recordBaseName = "omegalite";
  constexpr static const char * k_recordExtension = "clr";

  LoadStatus load(uint8_t * buffer, size_t capacity, size_t * sizeRead) override;
  bool save(const uint8_t * buffer, size_t size) override;
  bool clear() override;
};

} // namespace Colors
} // namespace OmegaLite

#endif
