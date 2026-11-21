#ifndef OMEGA_LITE_CUSTOM_COLORS_CELLS_H
#define OMEGA_LITE_CUSTOM_COLORS_CELLS_H

#include <escher.h>

#include "color_rgb.h"

namespace OmegaLite {
namespace Colors {

/* One customizable role: color swatch, label and current HEX value. */
class ColorRoleCell : public HighlightCell {
public:
  constexpr static KDCoordinate k_rowHeight = 26;

  ColorRoleCell();
  void setContent(const char * label, const char * hex, RgbColor color);
  void drawRect(KDContext * context, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() const override { return KDSize(0, k_rowHeight); }

private:
  const char * m_label;
  char m_hex[k_hexColorTextSize];
  RgbColor m_color;
};

/* A plain action row: save, cancel, restore. */
class ActionCell : public HighlightCell {
public:
  constexpr static KDCoordinate k_rowHeight = 30;

  ActionCell();
  void setText(const char * text) { m_text = text; }
  void drawRect(KDContext * context, KDRect rect) const override;
  KDSize minimalSizeForOptimalDisplay() const override { return KDSize(0, k_rowHeight); }

private:
  const char * m_text;
};

} // namespace Colors
} // namespace OmegaLite

#endif
