#include "color_store.h"

#include <string.h>

namespace OmegaLite {
namespace Colors {

MemoryColorStore::MemoryColorStore() : m_size(0), m_corrupted(false) {
  memset(m_buffer, 0, sizeof(m_buffer));
}

ColorStore::LoadStatus MemoryColorStore::load(uint8_t * buffer, size_t capacity, size_t * sizeRead) {
  if (sizeRead != nullptr) {
    *sizeRead = 0;
  }
  if (m_corrupted) {
    return LoadStatus::Corrupted;
  }
  if (m_size == 0) {
    return LoadStatus::Absent;
  }
  if (buffer == nullptr || capacity < m_size) {
    return LoadStatus::Corrupted;
  }
  memcpy(buffer, m_buffer, m_size);
  if (sizeRead != nullptr) {
    *sizeRead = m_size;
  }
  return LoadStatus::Loaded;
}

bool MemoryColorStore::save(const uint8_t * buffer, size_t size) {
  if (buffer == nullptr || size > sizeof(m_buffer) || size == 0) {
    return false;
  }
  memcpy(m_buffer, buffer, size);
  m_size = size;
  m_corrupted = false;
  return true;
}

bool MemoryColorStore::clear() {
  memset(m_buffer, 0, sizeof(m_buffer));
  m_size = 0;
  m_corrupted = false;
  return true;
}

} // namespace Colors
} // namespace OmegaLite
