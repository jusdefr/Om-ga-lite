#include "ion_color_store.h"

#include <ion/storage.h>
#include <string.h>

namespace OmegaLite {
namespace Colors {

namespace {

Ion::Storage * storage() { return Ion::Storage::sharedStorage(); }

Ion::Storage::Record colorRecord() {
  return storage()->recordBaseNamedWithExtension(IonColorStore::k_recordBaseName,
                                                 IonColorStore::k_recordExtension);
}

} // namespace

ColorStore::LoadStatus IonColorStore::load(uint8_t * buffer, size_t capacity, size_t * sizeRead) {
  if (sizeRead != nullptr) {
    *sizeRead = 0;
  }
  Ion::Storage::Record record = colorRecord();
  if (record.isNull()) {
    return LoadStatus::Absent;
  }
  Ion::Storage::Record::Data data = record.value();
  if (data.buffer == nullptr || data.size == 0 || data.size > capacity) {
    return LoadStatus::Corrupted;
  }
  memcpy(buffer, data.buffer, data.size);
  if (sizeRead != nullptr) {
    *sizeRead = data.size;
  }
  return LoadStatus::Loaded;
}

bool IonColorStore::save(const uint8_t * buffer, size_t size) {
  if (buffer == nullptr || size == 0 || size > UINT16_MAX) {
    return false;
  }
  Ion::Storage * recordStorage = storage();
  Ion::Storage::Record existing = colorRecord();
  if (!existing.isNull()) {
    const Ion::Storage::Record::Data current = existing.value();
    if (current.size == size && existing.setValue({buffer, size}) ==
                                    Ion::Storage::Record::ErrorStatus::None) {
      return true;
    }
    /* The serialized size changed (a role was added to or removed from the
     * overrides): Ion cannot rewrite a record with another size in place. */
    existing.destroy();
  }
  const Ion::Storage::Record::ErrorStatus status = recordStorage->createRecordWithExtension(
      k_recordBaseName, k_recordExtension, buffer, size);
  if (status != Ion::Storage::Record::ErrorStatus::None) {
    /* Do not leave a half written state behind: the next load will fall back
     * to the theme defaults. */
    Ion::Storage::Record failedRecord = colorRecord();
    if (!failedRecord.isNull()) {
      failedRecord.destroy();
    }
    return false;
  }
  return true;
}

bool IonColorStore::clear() {
  Ion::Storage::Record record = colorRecord();
  if (record.isNull()) {
    return true;
  }
  record.destroy();
  return colorRecord().isNull();
}

} // namespace Colors
} // namespace OmegaLite
