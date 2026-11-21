"""Tests for the runtime palette generator.

The generated pair (palette.h + palette_definitions.inc) is compiled and run
with a minimal KDColor stub, so the generated C++ itself is checked, not only
its text.
"""

import re
import shutil
import subprocess
import sys
import tempfile
import textwrap
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts"))

import generate_runtime_palette as generator  # noqa: E402

THEME_PATH = ROOT / "themes" / "omega_lite.json"

KDCOLOR_STUB = """\
#ifndef KANDINSKY_COLOR_H
#define KANDINSKY_COLOR_H
#include <stdint.h>
class KDColor {
public:
  constexpr KDColor() : m_value(0) {}
  static constexpr KDColor RGB24(uint32_t rgb) {
    return KDColor(((rgb & 0xF80000) >> 8) | ((rgb & 0x00FC00) >> 5) | ((rgb & 0x0000F8) >> 3));
  }
  constexpr uint16_t value() const { return m_value; }
  bool operator==(KDColor other) const { return m_value == other.m_value; }
  bool operator!=(KDColor other) const { return m_value != other.m_value; }
private:
  constexpr KDColor(uint16_t value) : m_value(value) {}
  uint16_t m_value;
};
constexpr KDColor KDColorBlack = KDColor::RGB24(0x000000);
constexpr KDColor KDColorBlue = KDColor::RGB24(0x0000FF);
#endif
"""

PROGRAM = """\
#include <escher/palette.h>
#include <escher/palette_definitions.inc>
#include <cstdio>

int main() {
  /* A theme color, a fixed color and an array must all be mutable. */
  Palette::PrimaryText = KDColorBlack;
  Palette::WallScreen = KDColorBlack;
  Palette::DataColor[0] = KDColorBlack;
  if (Palette::PrimaryText == ThemeDefaults::PrimaryText) return 1;
  Palette::resetToThemeDefaults();
  if (Palette::PrimaryText != ThemeDefaults::PrimaryText) return 2;
  if (Palette::WallScreen != ThemeDefaults::WallScreen) return 3;
  if (Palette::DataColor[0] != ThemeDefaults::DataColor[0]) return 4;
  /* nextDataColor wraps, and leaves an out of range index untouched. */
  int index = 0;
  KDColor first = Palette::nextDataColor(&index);
  if (first != Palette::DataColor[0] || index != 1) return 5;
  index = 999;
  if (Palette::nextDataColor(&index) != Palette::DataColor[0]) return 6;
  if (index != 999) return 7;
  std::printf("ok\\n");
  return 0;
}
"""


class RuntimePaletteTest(unittest.TestCase):
    def setUp(self):
        self.header, self.definitions, self.theme_color_count = generator.build(THEME_PATH)
        self.fixed_color_count = len(generator.EXTRA_COLORS) + 1  # + Cyan
        self.total = self.theme_color_count + self.fixed_color_count

    def test_theme_colors_are_counted(self):
        self.assertEqual(self.theme_color_count, 102)

    def scalar_members(self):
        return re.findall(r"^  static KDColor (\w+);$", self.header, re.MULTILINE)

    def test_header_declares_mutable_members(self):
        self.assertIn("class Palette", self.header)
        self.assertIn("static KDColor PrimaryText;", self.header)
        # Theme colors, the fixed colors and Cyan.
        self.assertEqual(len(self.scalar_members()),
                         self.theme_color_count + self.fixed_color_count)
        # Only ThemeDefaults keeps constexpr values.
        self.assertIn("constexpr static KDColor PrimaryText", self.header)

    def test_definitions_match_header(self):
        defined = re.findall(r"^KDColor Palette::(\w+) = ", self.definitions, re.MULTILINE)
        self.assertEqual(defined, self.scalar_members())

    def test_reset_restores_every_member(self):
        body = self.definitions.split("void Palette::resetToThemeDefaults()")[1]
        body = body.split("KDColor Palette::nextDataColor")[0]
        restored = re.findall(r"^  (\w+) = ThemeDefaults::\1;$", body, re.MULTILINE)
        self.assertEqual(restored, self.scalar_members())

    def test_theme_default_arrays_are_defined(self):
        # Indexing them in resetToThemeDefaults() odr-uses the arrays.
        self.assertIn("constexpr KDColor ThemeDefaults::DataColor[8];", self.definitions)
        self.assertIn("constexpr KDColor ThemeDefaults::DataColorLight[4];", self.definitions)
        self.assertIn("constexpr KDColor ThemeDefaults::AtomColor[11];", self.definitions)

    def test_arrays_have_explicit_sizes(self):
        self.assertIn("static KDColor DataColor[8];", self.header)
        self.assertIn("static KDColor DataColorLight[4];", self.header)
        self.assertIn("static KDColor AtomColor[11];", self.header)
        self.assertIn("numberOfDataColors() { return 8; }", self.header)

    def test_next_data_color_is_generated(self):
        self.assertIn("KDColor Palette::nextDataColor(int * colorIndex)", self.definitions)
        self.assertIn("static KDColor nextDataColor(int * colorIndex);", self.header)

    def test_theme_values_reach_the_header(self):
        self.assertIn("KDColor::RGB24(0xeef4ff)", self.header)
        self.assertIn("KDColor::RGB24(0x0b1220)", self.header)

    def test_generated_code_compiles_and_behaves(self):
        compiler = shutil.which("g++")
        if compiler is None:
            self.skipTest("g++ is not available")
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "escher").mkdir()
            (root / "kandinsky").mkdir()
            (root / "escher" / "palette.h").write_text(self.header, encoding="utf-8")
            (root / "escher" / "palette_definitions.inc").write_text(
                self.definitions, encoding="utf-8")
            (root / "kandinsky" / "color.h").write_text(KDCOLOR_STUB, encoding="utf-8")
            (root / "main.cpp").write_text(PROGRAM, encoding="utf-8")
            binary = root / "palette_test"
            compile_result = subprocess.run(
                [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                 f"-I{root}", str(root / "main.cpp"), "-o", str(binary)],
                capture_output=True, text=True,
            )
            self.assertEqual(compile_result.returncode, 0, compile_result.stderr)
            run_result = subprocess.run([str(binary)], capture_output=True, text=True)
            self.assertEqual(run_result.returncode, 0, run_result.stderr)


if __name__ == "__main__":
    unittest.main()
