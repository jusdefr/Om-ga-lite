#include "color_editor_view.h"

#include "preview_renderer.h"

namespace OmegaLite {
namespace Colors {

constexpr int ColorEditorView::k_numberOfRows;

namespace {

constexpr KDCoordinate k_rowHeight = 22;
constexpr KDCoordinate k_margin = 6;

void writeDecimal(uint8_t value, char * buffer) {
  /* Three digits, zero padded on the left, always terminated. */
  buffer[0] = static_cast<char>('0' + (value / 100) % 10);
  buffer[1] = static_cast<char>('0' + (value / 10) % 10);
  buffer[2] = static_cast<char>('0' + value % 10);
  buffer[3] = '\0';
}

} // namespace

ColorEditorView::ColorEditorView(const ColorSettings * settings, const Role * role,
                                 const int * selectedRow, const uint8_t * hexCursor) :
    m_settings(settings),
    m_role(role),
    m_selectedRow(selectedRow),
    m_hexCursor(hexCursor) {}

void ColorEditorView::drawRow(KDContext * context, KDCoordinate y, const char * label,
                              const char * value, bool selected) const {
  const KDCoordinate width = bounds().width();
  const KDColor background =
      selected ? Palette::ListCellBackgroundSelected : Palette::BackgroundApps;
  const KDColor labelColor = selected ? Palette::PrimaryText : Palette::SecondaryText;

  context->fillRect(KDRect(0, y, width, k_rowHeight), background);
  context->drawString(label, KDPoint(k_margin, y + 4), KDFont::SmallFont, labelColor, background);
  const KDCoordinate valueWidth = KDFont::SmallFont->stringSize(value).width();
  const KDCoordinate valueX = width - valueWidth - k_margin;
  if (valueX > k_margin) {
    context->drawString(value, KDPoint(valueX, y + 4), KDFont::SmallFont, Palette::AccentText,
                        background);
  }
  context->fillRect(KDRect(0, y + k_rowHeight - 1, width, 1), Palette::ListCellBorder);
}

void ColorEditorView::drawHexRow(KDContext * context, KDCoordinate y, const char * value) const {
  const KDCoordinate width = bounds().width();
  const bool selected = (*m_selectedRow == 0);
  const KDColor background =
      selected ? Palette::ListCellBackgroundSelected : Palette::BackgroundApps;

  context->fillRect(KDRect(0, y, width, k_rowHeight), background);
  context->drawString("HEX", KDPoint(k_margin, y + 4), KDFont::SmallFont,
                      selected ? Palette::PrimaryText : Palette::SecondaryText, background);

  const KDCoordinate glyphWidth = KDFont::SmallFont->glyphSize().width();
  const size_t length = 7; /* "#RRGGBB" */
  const KDCoordinate textWidth = static_cast<KDCoordinate>(length) * glyphWidth;
  KDCoordinate x = width - textWidth - k_margin;
  for (size_t index = 0; index < length; index++) {
    char character[2] = {value[index], '\0'};
    /* The dash and the selected nibble: nibble 0 is the first digit after '#'. */
    const int nibble = static_cast<int>(index) - 1;
    const bool isSelectedNibble =
        selected && nibble >= 0 && nibble == static_cast<int>(*m_hexCursor);
    const KDColor foreground = isSelectedNibble ? Palette::BackgroundApps : Palette::AccentText;
    const KDColor glyphBackground = isSelectedNibble ? Palette::AccentText : background;
    if (isSelectedNibble) {
      context->fillRect(KDRect(x, y + 3, glyphWidth, k_rowHeight - 6), glyphBackground);
    }
    context->drawString(character, KDPoint(x, y + 4), KDFont::SmallFont, foreground,
                        glyphBackground);
    x += glyphWidth;
  }
  context->fillRect(KDRect(0, y + k_rowHeight - 1, width, 1), Palette::ListCellBorder);
}

void ColorEditorView::drawRect(KDContext * context, KDRect rect) const {
  const KDCoordinate width = bounds().width();
  const KDCoordinate height = bounds().height();
  context->fillRect(KDRect(0, 0, width, height), Palette::BackgroundApps);

  const KDCoordinate reservedRows = k_numberOfRows * k_rowHeight + 2 * k_margin;
  KDCoordinate previewHeight = height - reservedRows - 18;
  if (previewHeight > 120) {
    previewHeight = 120;
  }
  if (previewHeight < 48) {
    previewHeight = 48;
  }

  drawPreview(context, KDRect(2, 2, width - 4, previewHeight), *m_settings->scheme());

  KDCoordinate y = previewHeight + 6;
  context->drawString(roleLabel(*m_role), KDPoint(k_margin, y + 2), KDFont::SmallFont,
                      Palette::AccentText, Palette::BackgroundApps);
  y += 16;

  const RgbColor current = m_settings->scheme()->color(*m_role);
  char hex[k_hexColorTextSize];
  writeHex(current, hex, sizeof(hex));
  char red[4];
  char green[4];
  char blue[4];
  writeDecimal(current.red(), red);
  writeDecimal(current.green(), green);
  writeDecimal(current.blue(), blue);

  drawHexRow(context, y, hex);
  y += k_rowHeight;
  drawRow(context, y, "Rouge", red, *m_selectedRow == 1);
  y += k_rowHeight;
  drawRow(context, y, "Vert", green, *m_selectedRow == 2);
  y += k_rowHeight;
  drawRow(context, y, "Bleu", blue, *m_selectedRow == 3);

  if (height > y + 14) {
    context->drawString("HEX: <> chiffre, +/- valeur   RGB: <> 1, +/- 16",
                        KDPoint(k_margin, height - 12), KDFont::SmallFont,
                        Palette::SecondaryText, Palette::BackgroundApps);
  }
}

} // namespace Colors
} // namespace OmegaLite
