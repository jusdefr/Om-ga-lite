#include "color_editor_controller.h"

#include <assert.h>

namespace OmegaLite {
namespace Colors {

ColorEditorController::ColorEditorController(Responder * parentResponder, ColorSettings * settings) :
    ViewController(parentResponder),
    m_settings(settings),
    m_role(Role::Background),
    m_selectedRow(k_hexRow),
    m_hexCursor(0),
    m_view(settings, &m_role, &m_selectedRow, &m_hexCursor) {
  assert(m_settings != nullptr);
}

const char * ColorEditorController::title() { return roleLabel(m_role); }

void ColorEditorController::setRole(Role role) {
  m_role = role;
  m_selectedRow = k_hexRow;
  m_hexCursor = 0;
  m_view.reload();
}

void ColorEditorController::viewWillAppear() {
  ViewController::viewWillAppear();
  m_view.reload();
}

void ColorEditorController::applyColor(RgbColor color) {
  m_settings->scheme()->setColor(m_role, color);
  m_view.reload();
}

void ColorEditorController::adjustChannel(int delta) {
  const uint8_t channel = static_cast<uint8_t>(m_selectedRow - 1);
  if (channel > 2) {
    return;
  }
  const RgbColor current = currentColor();
  applyColor(withChannel(current, channel, static_cast<int>(current.channel(channel)) + delta));
}

void ColorEditorController::adjustNibble(int delta) {
  if (m_hexCursor > 5) {
    return;
  }
  RgbColor current = currentColor();
  uint8_t channels[3] = {current.red(), current.green(), current.blue()};
  const uint8_t channel = m_hexCursor / 2;
  const bool highNibble = (m_hexCursor % 2) == 0;
  int nibble = highNibble ? (channels[channel] >> 4) : (channels[channel] & 0x0F);
  nibble = (nibble + delta + 16) % 16;
  channels[channel] = highNibble ? static_cast<uint8_t>((nibble << 4) | (channels[channel] & 0x0F))
                                 : static_cast<uint8_t>((channels[channel] & 0xF0) | nibble);
  applyColor(RgbColor(channels[0], channels[1], channels[2]));
}

void ColorEditorController::moveSelection(int delta) {
  int row = m_selectedRow + delta;
  if (row < k_hexRow) {
    row = k_hexRow;
  }
  if (row > k_lastRow) {
    row = k_lastRow;
  }
  if (row != m_selectedRow) {
    m_selectedRow = row;
    m_view.reload();
  }
}

bool ColorEditorController::handleEvent(Ion::Events::Event event) {
  if (event == Ion::Events::Up) {
    moveSelection(-1);
    return true;
  }
  if (event == Ion::Events::Down) {
    moveSelection(1);
    return true;
  }
  if (event == Ion::Events::Left) {
    if (m_selectedRow == k_hexRow) {
      m_hexCursor = (m_hexCursor + 5) % 6;
      m_view.reload();
    } else {
      adjustChannel(-1);
    }
    return true;
  }
  if (event == Ion::Events::Right) {
    if (m_selectedRow == k_hexRow) {
      m_hexCursor = (m_hexCursor + 1) % 6;
      m_view.reload();
    } else {
      adjustChannel(1);
    }
    return true;
  }
  if (event == Ion::Events::Plus) {
    if (m_selectedRow == k_hexRow) {
      adjustNibble(1);
    } else {
      adjustChannel(16);
    }
    return true;
  }
  if (event == Ion::Events::Minus) {
    if (m_selectedRow == k_hexRow) {
      adjustNibble(-1);
    } else {
      adjustChannel(-16);
    }
    return true;
  }
  /* Everything else, including Back, bubbles up to the stack view controller
   * so that it can pop this page. */
  return false;
}

} // namespace Colors
} // namespace OmegaLite
