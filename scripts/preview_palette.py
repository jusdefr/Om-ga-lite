#!/usr/bin/env python3
"""Render a preview of a theme and report text contrast.

This is the desktop counterpart of the on-screen live preview: it uses the same
role table as the firmware, so a theme whose colors are unreadable here will be
unreadable on the calculator. It writes a 320x240 PNG (the NumWorks screen
size) using only the Python standard library, and computes WCAG contrast ratios
for the pairs that matter for readability.

Usage:
    python3 scripts/preview_palette.py themes/omega_lite.json \
        --output build/preview/omega_lite.png

Exit codes: 0 success (warnings if contrast is low), 1 contrast below
--min-contrast, 2 usage or I/O error.
"""

from __future__ import annotations

import argparse
import json
import math
import struct
import sys
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from palette_schema import ROLE_PALETTE_KEYS  # noqa: E402

SCREEN_WIDTH = 320
SCREEN_HEIGHT = 240


def hex_to_rgb(value):
    value = value.lstrip("#")
    if len(value) != 6:
        raise ValueError(f"invalid color {value!r}")
    return tuple(int(value[i : i + 2], 16) for i in (0, 2, 4))


def flatten_theme_colors(colors, prefix=""):
    flat = {}
    for name, value in colors.items():
        if name == "":
            path = prefix
        else:
            path = name if not prefix else f"{prefix}.{name}"
        if isinstance(value, dict):
            flat.update(flatten_theme_colors(value, path))
        else:
            flat[path] = value
    return flat


def relative_luminance(rgb):
    channels = []
    for component in rgb:
        value = component / 255.0
        channels.append(value / 12.92 if value <= 0.03928 else ((value + 0.055) / 1.055) ** 2.4)
    red, green, blue = channels
    return 0.2126 * red + 0.7152 * green + 0.0722 * blue


def contrast_ratio(first, second):
    lighter = max(relative_luminance(first), relative_luminance(second))
    darker = min(relative_luminance(first), relative_luminance(second))
    return (lighter + 0.05) / (darker + 0.05)


class Canvas:
    def __init__(self, width, height, background):
        self.width = width
        self.height = height
        self.pixels = bytearray(bytes(background) * (width * height))

    def set_pixel(self, x, y, color):
        if 0 <= x < self.width and 0 <= y < self.height:
            offset = (y * self.width + x) * 3
            self.pixels[offset : offset + 3] = bytes(color)

    def fill_rect(self, x, y, width, height, color):
        left = max(0, x)
        right = min(self.width, x + width)
        if right <= left:
            return
        for row in range(max(0, y), min(self.height, y + height)):
            start = (row * self.width + left) * 3
            end = (row * self.width + right) * 3
            self.pixels[start:end] = bytes(color) * (right - left)

    def stroke_rect(self, x, y, width, height, color):
        self.fill_rect(x, y, width, 1, color)
        self.fill_rect(x, y + height - 1, width, 1, color)
        self.fill_rect(x, y, 1, height, color)
        self.fill_rect(x + width - 1, y, 1, height, color)

    def draw_line(self, x0, y0, x1, y1, color):
        dx = abs(x1 - x0)
        dy = -abs(y1 - y0)
        sx = 1 if x0 < x1 else -1
        sy = 1 if y0 < y1 else -1
        error = dx + dy
        while True:
            self.set_pixel(x0, y0, color)
            if x0 == x1 and y0 == y1:
                break
            doubled = 2 * error
            if doubled >= dy:
                error += dy
                x0 += sx
            if doubled <= dx:
                error += dx
                y0 += sy

    def draw_text_placeholder(self, x, y, width, color, thickness=3):
        """Three bars standing in for a line of text."""
        self.fill_rect(x, y, width, thickness, color)
        self.fill_rect(x, y + thickness + 2, max(6, width * 2 // 3), thickness, color)
        self.fill_rect(x, y + 2 * (thickness + 2), max(4, width // 2), thickness, color)

    def to_png(self):
        raw = bytearray()
        stride = self.width * 3
        for row in range(self.height):
            raw.append(0)  # filter type 0
            raw.extend(self.pixels[row * stride : (row + 1) * stride])

        def chunk(tag, payload):
            return (
                struct.pack(">I", len(payload))
                + tag
                + payload
                + struct.pack(">I", zlib.crc32(tag + payload) & 0xFFFFFFFF)
            )

        header = struct.pack(">IIBBBBB", self.width, self.height, 8, 2, 0, 0, 0)
        return (
            b"\x89PNG\r\n\x1a\n"
            + chunk(b"IHDR", header)
            + chunk(b"IDAT", zlib.compress(bytes(raw), 9))
            + chunk(b"IEND", b"")
        )


def theme_color(flat, path):
    try:
        return hex_to_rgb(flat[path])
    except KeyError:
        raise SystemExit(f"error: theme has no color {path!r}")


def render(flat):
    """Draw a miniature of the interface, one element per customizable role."""
    background = theme_color(flat, "Background.Hard")
    canvas = Canvas(SCREEN_WIDTH, SCREEN_HEIGHT, background)

    primary = theme_color(flat, "PrimaryText")
    secondary = theme_color(flat, "SecondaryText")
    accent = theme_color(flat, "AccentText")
    apps = theme_color(flat, "Background.Apps")
    secondary_background = theme_color(flat, "Background.AppsSecondary")
    toolbar = theme_color(flat, "Toolbar")
    toolbar_text = theme_color(flat, "Toolbar.Text")
    menu = theme_color(flat, "SubMenu.Background")
    button = theme_color(flat, "Button.Background")
    button_selected = theme_color(flat, "Button.BackgroundSelected")
    selection = theme_color(flat, "List.CellBackgroundSelected")
    border = theme_color(flat, "List.CellBorder")
    control = theme_color(flat, "Control.Enabled")
    curve = theme_color(flat, "Probability.Curve")
    grid = theme_color(flat, "Grid.PrimaryLine")
    axis = theme_color(flat, "Grid.SecondaryLine")

    # Toolbar
    canvas.fill_rect(0, 0, SCREEN_WIDTH, 22, toolbar)
    canvas.draw_text_placeholder(8, 6, 54, toolbar_text, thickness=2)
    canvas.fill_rect(SCREEN_WIDTH - 26, 6, 18, 9, toolbar_text)
    canvas.fill_rect(SCREEN_WIDTH - 22, 8, 10, 5, control)

    # Menu panel with a selected entry
    menu_width = 104
    canvas.fill_rect(0, 22, menu_width, 128, menu)
    for row in range(4):
        top = 28 + row * 26
        if row == 1:
            canvas.fill_rect(2, top - 3, menu_width - 5, 22, selection)
        canvas.draw_text_placeholder(10, top, 62, primary if row == 1 else secondary, thickness=2)
        canvas.fill_rect(6, top + 15, menu_width - 16, 1, border)
    canvas.fill_rect(menu_width - 1, 22, 1, 128, border)

    # Application area, inset so the screen background stays visible
    canvas.fill_rect(menu_width + 8, 30, SCREEN_WIDTH - menu_width - 16, 112, apps)
    canvas.draw_text_placeholder(menu_width + 12, 32, 92, primary, thickness=3)
    canvas.draw_text_placeholder(menu_width + 12, 56, 130, secondary, thickness=2)
    canvas.draw_text_placeholder(menu_width + 12, 74, 76, accent, thickness=2)

    # A control (slider) using the control color
    canvas.fill_rect(menu_width + 12, 94, 140, 3, border)
    canvas.fill_rect(menu_width + 12, 90, 40, 11, control)
    canvas.fill_rect(menu_width + 64, 90, 10, 11, secondary_background)
    canvas.stroke_rect(menu_width + 64, 90, 10, 11, border)

    # Two buttons, one selected
    canvas.fill_rect(menu_width + 12, 114, 70, 24, button)
    canvas.stroke_rect(menu_width + 12, 114, 70, 24, border)
    canvas.draw_text_placeholder(menu_width + 22, 122, 44, primary, thickness=2)
    canvas.fill_rect(menu_width + 92, 114, 70, 24, button_selected)
    canvas.draw_text_placeholder(menu_width + 102, 122, 44, primary, thickness=2)
    canvas.fill_rect(menu_width + 92, 114, 70, 2, accent)

    # Graph with grid, axes and a curve
    graph_top = 150
    graph_height = SCREEN_HEIGHT - graph_top
    canvas.fill_rect(6, graph_top + 6, SCREEN_WIDTH - 12, graph_height - 12, apps)
    for x in range(40, SCREEN_WIDTH, 40):
        canvas.fill_rect(x, graph_top + 8, 1, graph_height - 16, grid)
    for y in range(graph_top + 8, SCREEN_HEIGHT - 8, 20):
        canvas.fill_rect(10, y, SCREEN_WIDTH - 20, 1, grid)
    canvas.fill_rect(10, graph_top + 8, 1, graph_height - 16, axis)
    canvas.fill_rect(10, SCREEN_HEIGHT - 10, SCREEN_WIDTH - 20, 1, axis)

    previous = None
    for x in range(12, SCREEN_WIDTH - 12, 2):
        y = int(graph_top + 8 + (graph_height - 20) / 2 - 28 * math.sin((x - 12) / 30.0))
        if previous is not None:
            canvas.draw_line(previous[0], previous[1], x, y, curve)
        previous = (x, y)

    return canvas


def contrast_pairs():
    return [
        ("PrimaryText", "PrimaryText", "Background.Hard", 4.5),
        ("SecondaryText", "SecondaryText", "Background.Apps", 3.0),
        ("AccentText", "AccentText", "Background.Apps", 3.0),
        ("ToolbarText", "Toolbar.Text", "Toolbar", 4.5),
        ("ButtonText", "Button.Text", "Button.Background", 4.5),
        ("GraphCurve", "Probability.Curve", "Background.Apps", 3.0),
    ]


def report_contrast(flat):
    """Print ratios, return the names of the pairs below their threshold."""
    failures = []
    print("Contrast:")
    for name, foreground, background, required in contrast_pairs():
        ratio = contrast_ratio(theme_color(flat, foreground), theme_color(flat, background))
        status = "ok" if ratio >= required else f"LOW (needs {required})"
        if ratio < required:
            failures.append(name)
        print(f"  {name:<14} {ratio:5.2f}:1  {status}")
    if failures:
        print(f"warning: low contrast for: {', '.join(failures)}")
    return failures


def main(argv=None):
    parser = argparse.ArgumentParser(description="Render a theme preview PNG.")
    parser.add_argument("theme", help="theme JSON file")
    parser.add_argument("--output", help="output PNG path (default: build/preview/<theme>.png)")
    parser.add_argument(
        "--min-contrast",
        type=float,
        default=None,
        help="fail (exit 1) when a pair is below this ratio",
    )
    args = parser.parse_args(argv)

    with open(args.theme, "r", encoding="utf-8") as handle:
        data = json.load(handle)
    flat = flatten_theme_colors(data["colors"])

    missing = [key for key in ROLE_PALETTE_KEYS.values() if key not in flat]
    if missing:
        raise SystemExit("error: theme is missing keys: " + ", ".join(missing))

    output = (
        Path(args.output)
        if args.output
        else Path("build/preview") / (Path(args.theme).stem + ".png")
    )
    output.parent.mkdir(parents=True, exist_ok=True)
    canvas = render(flat)
    output.write_bytes(canvas.to_png())
    print(f"Wrote {output} ({canvas.width}x{canvas.height})")

    failures = report_contrast(flat)
    if args.min_contrast is not None and failures:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
