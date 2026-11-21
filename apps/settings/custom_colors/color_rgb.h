#ifndef OMEGA_LITE_CUSTOM_COLORS_COLOR_RGB_H
#define OMEGA_LITE_CUSTOM_COLORS_COLOR_RGB_H

#include <stddef.h>
#include <stdint.h>

namespace OmegaLite {
namespace Colors {

/* A plain 8-bit RGB color.
 *
 * Colors are stored as 24-bit RGB in the settings record. The conversion to
 * the 16-bit RGB565 format used by the screen is explicit (toRgb565), so that
 * this type stays independent from KDColor and the color logic can be unit
 * tested on a computer without the firmware toolchain. */
class RgbColor {
public:
  constexpr RgbColor() : m_red(0), m_green(0), m_blue(0) {}
  constexpr RgbColor(uint8_t red, uint8_t green, uint8_t blue) :
    m_red(red),
    m_green(green),
    m_blue(blue) {}
  constexpr uint8_t red() const { return m_red; }
  constexpr uint8_t green() const { return m_green; }
  constexpr uint8_t blue() const { return m_blue; }
  /* index must be 0, 1 or 2 (red, green, blue). */
  uint8_t channel(uint8_t index) const;
  static bool isChannelValue(int value) { return value >= 0 && value <= 255; }
  bool operator==(const RgbColor & other) const {
    return m_red == other.m_red && m_green == other.m_green && m_blue == other.m_blue;
  }
  bool operator!=(const RgbColor & other) const { return !(*this == other); }
private:
  uint8_t m_red;
  uint8_t m_green;
  uint8_t m_blue;
};

/* Size of the buffer required by writeHex: "#RRGGBB" plus the terminator. */
constexpr size_t k_hexColorTextSize = 8;
/* Size of the buffer required by writeThemeHex: "RRGGBB" plus the terminator.
 * Omega theme files store colors without the leading dash. */
constexpr size_t k_themeHexColorTextSize = 7;

/* Parses "#RRGGBB" or "RRGGBB" (case insensitive).
 * Returns false on any malformed input: wrong length, non hexadecimal
 * character, empty string or null pointer. Nothing is written on failure. */
bool parseHex(const char * text, RgbColor * result);

/* Writes "#RRGGBB" (uppercase) into buffer. If bufferSize is too small the
 * buffer receives an empty string. Returns false in that case. */
bool writeHex(RgbColor color, char * buffer, size_t bufferSize);

/* Writes "RRGGBB" (uppercase, no dash), the format used by Omega themes. */
bool writeThemeHex(RgbColor color, char * buffer, size_t bufferSize);

/* 24-bit RGB to RGB565, matching KDColor::RGB888 from kandinsky. */
uint16_t toRgb565(RgbColor color);
/* RGB565 back to 8-bit RGB, matching KDColor::red()/green()/blue().
 * The low bits are lost: this is the exact value the screen can display. */
RgbColor fromRgb565(uint16_t color);

/* Returns a copy of color with one channel replaced. Values outside 0..255
 * are clamped instead of wrapping. */
RgbColor withChannel(RgbColor color, uint8_t index, int value);

/* Relative luminance in 0..255, used to pick readable text over a color. */
uint8_t luminance(RgbColor color);

} // namespace Colors
} // namespace OmegaLite

#endif
