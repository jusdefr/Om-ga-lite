#ifndef OMEGA_LITE_CUSTOM_COLORS_EDITOR_CONTROLLER_H
#define OMEGA_LITE_CUSTOM_COLORS_EDITOR_CONTROLLER_H

#include <escher.h>

#include "color_editor_view.h"
#include "color_settings.h"

namespace OmegaLite {
namespace Colors {

/* Edits one role: live preview plus HEX nibble and per channel editing.
 *
 * Keys:
 *   Up/Down     move between the HEX row and the R, G, B rows
 *   Left/Right  on HEX: select a digit; on a channel: subtract/add 1
 *   Plus/Minus  on HEX: change the selected digit; on a channel: subtract/add 16
 *   Back        return to the list, keeping the draft for the preview
 *
 * Nothing is written to storage here: saving is an explicit action of the
 * customization page, which is what makes "cancel" possible. */
class ColorEditorController : public ViewController {
public:
  ColorEditorController(Responder * parentResponder, ColorSettings * settings);
  View * view() override { return &m_view; }
  const char * title() override;
  void viewWillAppear() override;
  bool handleEvent(Ion::Events::Event event) override;

  void setRole(Role role);
  Role role() const { return m_role; }

  const ColorSettings * settings() const { return m_settings; }
  int selectedRow() const { return m_selectedRow; }
  uint8_t hexCursor() const { return m_hexCursor; }

private:
  enum { k_hexRow = 0, k_redRow = 1, k_greenRow = 2, k_blueRow = 3, k_lastRow = k_blueRow };

  RgbColor currentColor() const { return m_settings->scheme()->color(m_role); }
  void applyColor(RgbColor color);
  void adjustChannel(int delta);
  void adjustNibble(int delta);
  void moveSelection(int delta);

  ColorSettings * m_settings;
  Role m_role;
  int m_selectedRow;
  uint8_t m_hexCursor;
  ColorEditorView m_view;
};

} // namespace Colors
} // namespace OmegaLite

#endif
