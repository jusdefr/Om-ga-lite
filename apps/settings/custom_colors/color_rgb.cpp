#include "color_rgb.h"

namespace OmegaLite {
namespace Colors {

namespace {

constexpr int hexDigitValue(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  return -1;
}

constexpr char k_hexDigits[] = "0123456789ABCDEF";

void writeByte(uint8_t value, char * output) {
  output[0] = k_hexDigits[(value >> 4) & 0xF];
  output[1] = k_hexDigits[value & 0xF];
}

} // namespace

uint8_t RgbColor::channel(uint8_t index) const {
  if (index == 0) {
    return m_red;
  }
  if (index == 1) {
    return m_green;
  }
  return m_blue;
}

bool parseHex(const char * text, RgbColor * result) {
  if (text == nullptr || result == nullptr) {
    return false;
  }
  const char * digits = text;
  if (*digits == '#') {
    digits++;
  }
  /* Exactly six hexadecimal digits: no shortcut form, no trailing character. */
  uint8_t channels[3] = {0, 0, 0};
  for (int i = 0; i < 6; i++) {
    int value = hexDigitValue(digits[i]);
    if (value < 0) {
      return false;
    }
    channels[i / 2] = static_cast<uint8_t>((channels[i / 2] << 4) | value);
  }
  if (digits[6] != '\0') {
    return false;
  }
  *result = RgbColor(channels[0], channels[1], channels[2]);
  return true;
}

bool writeHex(RgbColor color, char * buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize < k_hexColorTextSize) {
    if (buffer != nullptr && bufferSize > 0) {
      buffer[0] = '\0';
    }
    return false;
  }
  buffer[0] = '#';
  writeByte(color.red(), buffer + 1);
  writeByte(color.green(), buffer + 3);
  writeByte(color.blue(), buffer + 5);
  buffer[7] = '\0';
  return true;
}

bool writeThemeHex(RgbColor color, char * buffer, size_t bufferSize) {
  if (buffer == nullptr || bufferSize < k_themeHexColorTextSize) {
    if (buffer != nullptr && bufferSize > 0) {
      buffer[0] = '\0';
    }
    return false;
  }
  writeByte(color.red(), buffer);
  writeByte(color.green(), buffer + 2);
  writeByte(color.blue(), buffer + 4);
  buffer[6] = '\0';
  return true;
}

uint16_t toRgb565(RgbColor color) {
  return static_cast<uint16_t>(((color.red() >> 3) << 11) |
                               ((color.green() >> 2) << 5) |
                               (color.blue() >> 3));
}

RgbColor fromRgb565(uint16_t color) {
  const uint8_t red = static_cast<uint8_t>(((color >> 11) & 0x1F) << 3);
  const uint8_t green = static_cast<uint8_t>(((color >> 5) & 0x3F) << 2);
  const uint8_t blue = static_cast<uint8_t>((color & 0x1F) << 3);
  return RgbColor(red, green, blue);
}

RgbColor withChannel(RgbColor color, uint8_t index, int value) {
  if (value < 0) {
    value = 0;
  }
  if (value > 255) {
    value = 255;
  }
  const uint8_t channel = static_cast<uint8_t>(value);
  if (index == 0) {
    return RgbColor(channel, color.green(), color.blue());
  }
  if (index == 1) {
    return RgbColor(color.red(), channel, color.blue());
  }
  return RgbColor(color.red(), color.green(), channel);
}

uint8_t luminance(RgbColor color) {
  /* Integer approximation of 0.299R + 0.587G + 0.114B. */
  const uint32_t value = 299u * color.red() + 587u * color.green() + 114u * color.blue();
  return static_cast<uint8_t>(value / 1000u);
}

} // namespace Colors
} // namespace OmegaLite
