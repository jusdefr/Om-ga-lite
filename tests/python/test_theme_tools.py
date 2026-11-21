"""Tests for the theme validator and the palette preview renderer.

Run with:  python3 -m unittest discover -s tests/python -v
"""

import json
import re
import struct
import sys
import tempfile
import unittest
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

import palette_schema  # noqa: E402
import preview_palette  # noqa: E402
import validate_theme  # noqa: E402

THEME_PATH = ROOT / "themes" / "omega_lite.json"


def load_theme():
    with open(THEME_PATH, "r", encoding="utf-8") as handle:
        return json.load(handle)


def mutate(mutator):
    """Return a deep-copied theme after applying mutator(data)."""
    data = load_theme()
    mutator(data)
    return data


def decode_png(data):
    """Decode the 8-bit RGB, filter-0 PNG written by preview_palette."""
    assert data[:8] == b"\x89PNG\r\n\x1a\n"
    offset = 8
    width = height = None
    idat = b""
    while offset < len(data):
        length = struct.unpack(">I", data[offset : offset + 4])[0]
        tag = data[offset + 4 : offset + 8]
        payload = data[offset + 8 : offset + 8 + length]
        offset += 12 + length
        if tag == b"IHDR":
            width, height, depth, color_type = struct.unpack(">IIBB", payload[:10])
            assert depth == 8 and color_type == 2
        elif tag == b"IDAT":
            idat += payload
        elif tag == b"IEND":
            break
    raw = zlib.decompress(idat)
    stride = width * 3
    pixels = []
    for row in range(height):
        start = row * (stride + 1)
        assert raw[start] == 0, "unexpected PNG filter"
        line = raw[start + 1 : start + 1 + stride]
        pixels.append([tuple(line[i : i + 3]) for i in range(0, stride, 3)])
    return width, height, pixels


class ThemeFileTest(unittest.TestCase):
    def test_shipped_theme_is_valid(self):
        self.assertEqual(validate_theme.validate_theme_data(load_theme()), [])

    def test_shipped_theme_has_every_role(self):
        errors = []
        flat = validate_theme.flatten_colors(load_theme()["colors"], errors)
        self.assertEqual(errors, [])
        self.assertEqual(validate_theme.missing_role_keys(flat), [])

    def test_shipped_theme_has_the_expected_key_count(self):
        errors = []
        flat = validate_theme.flatten_colors(load_theme()["colors"], errors)
        self.assertEqual(len(flat), len(palette_schema.expected_paths()))
        self.assertEqual(len(flat), 102)

    def test_theme_name(self):
        self.assertEqual(load_theme()["name"], "Omega Lite")

    def test_icons_reference_an_upstream_icon_set(self):
        # Reusing an existing icon folder avoids shipping derived artwork.
        self.assertEqual(load_theme()["icons"], "epsilon_dark")

    def test_missing_key_is_reported(self):
        data = mutate(lambda theme: theme["colors"]["Background"].pop("Apps"))
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any("Background.Apps" in error for error in errors), errors)

    def test_unknown_key_is_reported(self):
        data = mutate(lambda theme: theme["colors"].update({"PrimaryTxt": "ffffff"}))
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any("unknown color key" in error for error in errors), errors)

    def test_unknown_key_gets_a_suggestion(self):
        data = mutate(lambda theme: theme["colors"].update({"PrimaryTxt": "ffffff"}))
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any("PrimaryText" in error for error in errors), errors)

    def test_hash_prefixed_color_is_rejected(self):
        data = mutate(lambda theme: theme["colors"].update({"PrimaryText": "#eef4ff"}))
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any("six digit hexadecimal" in error for error in errors), errors)

    def test_short_color_is_rejected(self):
        data = mutate(lambda theme: theme["colors"].update({"PrimaryText": "fff"}))
        self.assertTrue(validate_theme.validate_theme_data(data))

    def test_non_string_color_is_rejected(self):
        data = mutate(lambda theme: theme["colors"].update({"PrimaryText": 0xEEF4FF}))
        self.assertTrue(validate_theme.validate_theme_data(data))

    def test_missing_name_is_reported(self):
        data = mutate(lambda theme: theme.pop("name"))
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any('"name"' in error for error in errors), errors)

    def test_colors_must_be_an_object(self):
        data = mutate(lambda theme: theme.update({"colors": []}))
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any('"colors" must be a JSON object' in error for error in errors), errors)

    def test_non_object_theme(self):
        self.assertEqual(
            validate_theme.validate_theme_data(["not", "a", "theme"]),
            ["the theme must be a JSON object"],
        )

    def test_group_color_is_flattened_without_suffix(self):
        errors = []
        flat = validate_theme.flatten_colors(load_theme()["colors"], errors)
        self.assertIn("Toolbar", flat)
        self.assertIn("Toolbar.Text", flat)
        self.assertNotIn("Toolbar.", flat)

    def test_deep_group_is_rejected(self):
        data = mutate(
            lambda theme: theme["colors"]["Background"].update({"Nested": {"Deep": "000000"}})
        )
        errors = validate_theme.validate_theme_data(data)
        self.assertTrue(any("nested this deeply" in error for error in errors), errors)

    def test_command_line_accepts_the_shipped_theme(self):
        self.assertEqual(validate_theme.main([str(THEME_PATH), "--check-roles"]), 0)

    def test_command_line_rejects_an_invalid_theme(self):
        data = mutate(lambda theme: theme["colors"].update({"PrimaryText": "xyz"}))
        with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as handle:
            json.dump(data, handle)
            path = handle.name
        try:
            self.assertEqual(validate_theme.main([path]), 1)
        finally:
            Path(path).unlink()


class SchemaConsistencyTest(unittest.TestCase):
    def test_schema_paths_are_unique(self):
        paths = palette_schema.expected_paths()
        self.assertEqual(len(paths), len(set(paths)))

    def test_role_keys_belong_to_the_schema(self):
        valid = set(palette_schema.expected_paths())
        for role, key in palette_schema.ROLE_PALETTE_KEYS.items():
            self.assertIn(key, valid, f"role {role} maps to unknown key {key}")

    def test_role_keys_are_unique(self):
        keys = list(palette_schema.ROLE_PALETTE_KEYS.values())
        self.assertEqual(len(keys), len(set(keys)))

    def test_python_role_table_matches_cpp(self):
        """The firmware table and the tooling table must not drift apart."""
        source = (ROOT / "apps" / "settings" / "custom_colors" / "color_roles.cpp").read_text()
        block = source[source.index("k_roleDefinitions") : source.index("};", source.index("k_roleDefinitions"))]
        entries = re.findall(r'\{"([^"]+)",\s*"([^"]*)",\s*"([^"]*)"\}', block)
        self.assertEqual(len(entries), 16)
        identifiers = [entry[0] for entry in entries]
        palette_keys = [entry[2] for entry in entries]
        self.assertEqual(identifiers, list(palette_schema.ROLE_PALETTE_KEYS.keys()))
        self.assertEqual(palette_keys, list(palette_schema.ROLE_PALETTE_KEYS.values()))


class GeneratedDefaultsTest(unittest.TestCase):
    def test_generated_header_matches_the_theme(self):
        sys.path.insert(0, str(ROOT / "scripts"))
        import generate_palette_defaults as generator

        header = (ROOT / "apps" / "settings" / "custom_colors"
                  / "palette_defaults_generated.h").read_text(encoding="utf-8")
        self.assertEqual(header, generator.build_header(THEME_PATH))

    def test_generated_header_has_every_role(self):
        header = (ROOT / "apps" / "settings" / "custom_colors"
                  / "palette_defaults_generated.h").read_text(encoding="utf-8")
        self.assertEqual(header.count("RgbColor(0x"), 16)


class PreviewTest(unittest.TestCase):
    def test_contrast_ratio_extremes(self):
        black = (0, 0, 0)
        white = (255, 255, 255)
        self.assertAlmostEqual(preview_palette.contrast_ratio(black, white), 21.0, places=2)
        self.assertAlmostEqual(preview_palette.contrast_ratio(white, black), 21.0, places=2)
        self.assertAlmostEqual(preview_palette.contrast_ratio(black, black), 1.0, places=2)

    def test_shipped_theme_has_acceptable_contrast(self):
        flat = preview_palette.flatten_theme_colors(load_theme()["colors"])
        self.assertEqual(preview_palette.report_contrast(flat), [])

    def test_preview_image_is_320x240(self):
        errors = []
        flat = validate_theme.flatten_colors(load_theme()["colors"], errors)
        canvas = preview_palette.render(flat)
        width, height, pixels = decode_png(canvas.to_png())
        self.assertEqual((width, height), (320, 240))
        self.assertEqual(len(pixels), 240)
        self.assertEqual(len(pixels[0]), 320)

    def test_every_role_is_drawn_in_the_preview(self):
        errors = []
        flat = validate_theme.flatten_colors(load_theme()["colors"], errors)
        canvas = preview_palette.render(flat)
        _, _, pixels = decode_png(canvas.to_png())
        drawn = {pixel for row in pixels for pixel in row}
        for role, key in palette_schema.ROLE_PALETTE_KEYS.items():
            self.assertIn(preview_palette.hex_to_rgb(flat[key]), drawn, f"role {role} ({key}) not drawn")

    def test_preview_writes_a_png_file(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "preview.png"
            code = preview_palette.main([str(THEME_PATH), "--output", str(output)])
            self.assertEqual(code, 0)
            self.assertTrue(output.exists())
            self.assertEqual(output.read_bytes()[:8], b"\x89PNG\r\n\x1a\n")


if __name__ == "__main__":
    unittest.main()
