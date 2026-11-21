#include "preview_renderer.h"

#include "palette_bridge.h"

namespace OmegaLite {
namespace Colors {

namespace {

/* Quarter period sine table, scaled by 100, enough to draw a readable curve
 * without pulling libm into the drawing path. */
constexpr int k_sineLength = 17;
constexpr int8_t k_sine[k_sineLength] = {
  0, 38, 71, 92, 100, 92, 71, 38, 0, -38, -71, -92, -100, -92, -71, -38, 0
};

KDColor colorOf(const ColorScheme & scheme, Role role) {
  return toKDColor(scheme.color(role));
}

} // namespace

void drawPreview(KDContext * context, KDRect rect, const ColorScheme & scheme) {
  const KDCoordinate width = rect.width();
  const KDCoordinate height = rect.height();
  if (width < 40 || height < 40) {
    return;
  }

  /* Background */
  context->fillRect(KDRect(0, 0, width, height), colorOf(scheme, Role::Background));

  /* Toolbar */
  const KDCoordinate toolbarHeight = height / 8 < 10 ? 10 : height / 8;
  context->fillRect(KDRect(0, 0, width, toolbarHeight), colorOf(scheme, Role::Toolbar));
  context->fillRect(KDRect(4, toolbarHeight / 2 - 1, width / 6, 3),
                    colorOf(scheme, Role::PrimaryText));

  const KDCoordinate graphHeight = height / 3;
  const KDCoordinate contentTop = toolbarHeight;
  const KDCoordinate contentBottom = height - graphHeight;
  const KDCoordinate menuWidth = width / 3;

  /* Menu with a selected entry */
  context->fillRect(KDRect(0, contentTop, menuWidth, contentBottom - contentTop),
                    colorOf(scheme, Role::MenuBackground));
  context->fillRect(KDRect(menuWidth - 1, contentTop, 1, contentBottom - contentTop),
                    colorOf(scheme, Role::Border));
  const KDCoordinate itemHeight = (contentBottom - contentTop) / 3;
  for (int item = 0; item < 3 && itemHeight > 4; item++) {
    const KDCoordinate top = contentTop + item * itemHeight;
    if (item == 1) {
      context->fillRect(KDRect(1, top + 1, menuWidth - 3, itemHeight - 2),
                        colorOf(scheme, Role::Selection));
    }
    context->fillRect(KDRect(4, top + 5, menuWidth - 14, 3),
                      colorOf(scheme, item == 1 ? Role::PrimaryText : Role::SecondaryText));
    context->fillRect(KDRect(0, top + itemHeight - 1, menuWidth, 1),
                      colorOf(scheme, Role::Border));
  }

  /* Application card */
  const KDCoordinate appLeft = menuWidth + 4;
  const KDCoordinate appWidth = width - appLeft - 4;
  if (appWidth < 24) {
    return;
  }
  context->fillRect(KDRect(appLeft, contentTop + 3, appWidth, contentBottom - contentTop - 6),
                    colorOf(scheme, Role::AppBackground));
  context->fillRect(KDRect(appLeft + 6, contentTop + 8, appWidth / 2, 4),
                    colorOf(scheme, Role::PrimaryText));
  context->fillRect(KDRect(appLeft + 6, contentTop + 16, appWidth * 2 / 3, 3),
                    colorOf(scheme, Role::SecondaryText));
  context->fillRect(KDRect(appLeft + 6, contentTop + 22, appWidth / 3, 3),
                    colorOf(scheme, Role::AccentText));

  /* Control and buttons */
  const KDCoordinate sliderY = contentBottom - 26;
  context->fillRect(KDRect(appLeft + 6, sliderY, appWidth / 2, 2), colorOf(scheme, Role::Border));
  context->fillRect(KDRect(appLeft + 6, sliderY - 3, appWidth / 6, 8),
                    colorOf(scheme, Role::Control));
  const KDCoordinate buttonWidth = (appWidth - 18) / 2;
  if (buttonWidth > 10) {
    context->fillRect(KDRect(appLeft + 6, sliderY + 8, buttonWidth, 10),
                      colorOf(scheme, Role::Button));
    context->strokeRect(KDRect(appLeft + 6, sliderY + 8, buttonWidth, 10),
                        colorOf(scheme, Role::Border));
    context->fillRect(KDRect(appLeft + 12 + buttonWidth, sliderY + 8, buttonWidth, 10),
                      colorOf(scheme, Role::ButtonSelected));
  }

  /* Graph */
  const KDCoordinate graphTop = contentBottom;
  context->fillRect(KDRect(0, graphTop, width, graphHeight),
                    colorOf(scheme, Role::AppBackground));
  for (KDCoordinate x = 16; x < width - 2; x += 16) {
    context->fillRect(KDRect(x, graphTop + 2, 1, graphHeight - 6),
                      colorOf(scheme, Role::GraphGrid));
  }
  for (KDCoordinate y = graphTop + 8; y < height - 3; y += 12) {
    context->fillRect(KDRect(4, y, width - 8, 1), colorOf(scheme, Role::GraphGrid));
  }
  context->fillRect(KDRect(4, graphTop + 2, 1, graphHeight - 6),
                    colorOf(scheme, Role::GraphAxis));
  context->fillRect(KDRect(4, height - 3, width - 8, 1), colorOf(scheme, Role::GraphAxis));

  /* Curve, sampled with the sine table */
  const KDCoordinate usableHeight = graphHeight - 10;
  const KDCoordinate amplitude = usableHeight / 2;
  const KDCoordinate center = graphTop + graphHeight / 2;
  const KDCoordinate span = width > 12 ? width - 12 : 1;
  bool hasPrevious = false;
  KDPoint previous(0, 0);
  for (KDCoordinate x = 6; x < width - 6; x += 2) {
    const uint32_t index = static_cast<uint32_t>((x - 6) * (k_sineLength - 1)) / span;
    const KDCoordinate y = center - (amplitude * k_sine[index]) / 100;
    const KDPoint point(x, y);
    if (hasPrevious) {
      context->drawLine(previous, point, colorOf(scheme, Role::GraphCurve));
    }
    previous = point;
    hasPrevious = true;
  }
}

} // namespace Colors
} // namespace OmegaLite
