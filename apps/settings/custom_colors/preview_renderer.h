#ifndef OMEGA_LITE_CUSTOM_COLORS_PREVIEW_RENDERER_H
#define OMEGA_LITE_CUSTOM_COLORS_PREVIEW_RENDERER_H

#include <kandinsky.h>

#include "color_scheme.h"

namespace OmegaLite {
namespace Colors {

/* Draws a miniature of the interface at rect, in the current scheme colors:
 * toolbar, menu with a selected row, application card with texts, a control,
 * two buttons and a graph with grid, axes and a curve.
 *
 * Used by the live preview of the editor and by the preview cell. It reads the
 * scheme on every draw, so calling it again after changing a color is the
 * preview. */
void drawPreview(KDContext * context, KDRect rect, const ColorScheme & scheme);

} // namespace Colors
} // namespace OmegaLite

#endif
