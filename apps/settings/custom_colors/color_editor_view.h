#ifndef OMEGA_LITE_CUSTOM_COLORS_EDITOR_VIEW_H
#define OMEGA_LITE_CUSTOM_COLORS_EDITOR_VIEW_H

#include <escher.h>

#include "color_settings.h"

namespace OmegaLite {
namespace Colors {

/* The editor screen: the live preview on top, then the HEX value and the
 * three channels. The view reads the controller state through pointers, which
 * keeps one source of truth for the selection and the edited color. */
class ColorEditorView : public View {
public:
  ColorEditorView(const ColorSettings * settings, const Role * role, const int * selectedRow,
                  const uint8_t * hexCursor);
  void drawRect(KDContext * context, KDRect rect) const override;
  /* Redraws the whole view; called after every color change. */
  void reload() { markRectAsDirty(bounds()); }

  constexpr static int k_numberOfRows = 4;

private:
  void drawRow(KDContext * context, KDCoordinate y, const char * label, const char * value,
               bool selected) const;
  void drawHexRow(KDContext * context, KDCoordinate y, const char * value) const;

  const ColorSettings * m_settings;
  const Role * m_role;
  const int * m_selectedRow;
  const uint8_t * m_hexCursor;
};

} // namespace Colors
} // namespace OmegaLite

#endif
