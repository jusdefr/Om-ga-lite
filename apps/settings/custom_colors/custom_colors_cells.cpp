#include "custom_colors_cells.h"

#include "palette_bridge.h"

namespace OmegaLite {
namespace Colors {

ColorRoleCell::ColorRoleCell() : m_label(""), m_hex{0}, m_color() {}

void ColorRoleCell::setContent(const char * label, const char * hex, RgbColor color) {
  m_label = label;
  if (hex != nullptr) {
    size_t index = 0;
    while (index + 1 < sizeof(m_hex) && hex[index] != '\0') {
      m_hex[index] = hex[index];
      index++;
    }
    m_hex[index] = '\0';
  } else {
    m_hex[0] = '\0';
  }
  m_color = color;
}

void ColorRoleCell::drawRect(KDContext * context, KDRect rect) const {
  const KDCoordinate width = bounds().width();
  const KDCoordinate height = bounds().height();
  const KDColor background =
      m_highlighted ? Palette::ListCellBackgroundSelected : Palette::ListCellBackground;

  context->fillRect(KDRect(0, 0, width, height), background);

  /* Swatch, with a border so that very light colors stay visible on white. */
  context->fillRect(KDRect(6, 5, 16, 16), toKDColor(m_color));
  context->strokeRect(KDRect(6, 5, 16, 16), Palette::ListCellBorder);

  context->drawString(m_label, KDPoint(28, 4), KDFont::SmallFont, Palette::PrimaryText, background);

  const KDCoordinate hexWidth = KDFont::SmallFont->stringSize(m_hex).width();
  const KDCoordinate hexX = width - hexWidth - 6;
  if (hexX > 28) {
    context->drawString(m_hex, KDPoint(hexX, 4), KDFont::SmallFont, Palette::AccentText, background);
  }

  context->fillRect(KDRect(0, height - 1, width, 1), Palette::ListCellBorder);
}

ActionCell::ActionCell() : m_text("") {}

void ActionCell::drawRect(KDContext * context, KDRect rect) const {
  const KDCoordinate width = bounds().width();
  const KDCoordinate height = bounds().height();
  const KDColor background =
      m_highlighted ? Palette::ListCellBackgroundSelected : Palette::BackgroundApps;

  context->fillRect(KDRect(0, 0, width, height), background);
  const KDCoordinate textWidth = KDFont::SmallFont->stringSize(m_text).width();
  const KDCoordinate x = (width - textWidth) / 2;
  context->drawString(m_text, KDPoint(x > 0 ? x : 4, 7), KDFont::SmallFont,
                      m_highlighted ? Palette::PrimaryText : Palette::AccentText, background);
  context->fillRect(KDRect(0, height - 1, width, 1), Palette::ListCellBorder);
}

} // namespace Colors
} // namespace OmegaLite
