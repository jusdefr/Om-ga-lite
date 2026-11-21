#include <string>

#include "test_framework.h"

#include "color_rgb.h"

using namespace OmegaLite::Colors;

TEST(HexParsingAcceptsDashPrefix) {
  RgbColor color;
  CHECK(parseHex("#0B1220", &color));
  CHECK_EQ(color.red(), 0x0B);
  CHECK_EQ(color.green(), 0x12);
  CHECK_EQ(color.blue(), 0x20);
}

TEST(HexParsingAcceptsBareDigits) {
  RgbColor color;
  CHECK(parseHex("0b1220", &color));
  CHECK_EQ(color.red(), 0x0B);
  CHECK_EQ(color.green(), 0x12);
  CHECK_EQ(color.blue(), 0x20);
}

TEST(HexParsingIsCaseInsensitive) {
  RgbColor lower;
  RgbColor upper;
  CHECK(parseHex("#abcdef", &lower));
  CHECK(parseHex("#ABCDEF", &upper));
  CHECK(lower == upper);
  CHECK_EQ(lower.red(), 0xAB);
  CHECK_EQ(lower.green(), 0xCD);
  CHECK_EQ(lower.blue(), 0xEF);
}

TEST(HexParsingRejectsMalformedInput) {
  RgbColor color(1, 2, 3);
  CHECK(!parseHex("", &color));
  CHECK(!parseHex("#", &color));
  CHECK(!parseHex("#12345", &color));
  CHECK(!parseHex("#1234567", &color));
  CHECK(!parseHex("12345", &color));
  CHECK(!parseHex("#12345G", &color));
  CHECK(!parseHex("# 12345", &color));
  CHECK(!parseHex(" 123456", &color));
  CHECK(!parseHex("0x1234", &color));
  CHECK(!parseHex(nullptr, &color));
  CHECK(!parseHex("#123456", nullptr));
  /* A failed parse must not modify the output. */
  CHECK(color == RgbColor(1, 2, 3));
}

TEST(HexFormattingUsesUppercaseWithDash) {
  char buffer[k_hexColorTextSize];
  CHECK(writeHex(RgbColor(0x0B, 0x12, 0x20), buffer, sizeof(buffer)));
  CHECK_EQ(std::string(buffer), std::string("#0B1220"));
}

TEST(HexFormattingFailsOnSmallBuffer) {
  char buffer[k_hexColorTextSize];
  buffer[0] = 'x';
  CHECK(!writeHex(RgbColor(1, 2, 3), buffer, 4));
  CHECK_EQ(buffer[0], '\0');
}

TEST(ThemeHexFormattingHasNoDash) {
  char buffer[k_themeHexColorTextSize];
  CHECK(writeThemeHex(RgbColor(0x2F, 0xD3, 0xC4), buffer, sizeof(buffer)));
  CHECK_EQ(std::string(buffer), std::string("2FD3C4"));
}

TEST(HexRoundTrip) {
  const char * texts[] = {"#000000", "#FFFFFF", "#2FD3C4", "#0B1220", "#FE871F"};
  for (const char * text : texts) {
    RgbColor color;
    CHECK(parseHex(text, &color));
    char buffer[k_hexColorTextSize];
    CHECK(writeHex(color, buffer, sizeof(buffer)));
    CHECK_EQ(std::string(buffer), std::string(text));
  }
}

TEST(Rgb565MatchesScreenQuantization) {
  CHECK_EQ(toRgb565(RgbColor(0, 0, 0)), 0x0000);
  CHECK_EQ(toRgb565(RgbColor(255, 255, 255)), 0xFFFF);
  CHECK_EQ(toRgb565(RgbColor(255, 0, 0)), 0xF800);
  CHECK_EQ(toRgb565(RgbColor(0, 255, 0)), 0x07E0);
  CHECK_EQ(toRgb565(RgbColor(0, 0, 255)), 0x001F);
  /* Intermediate values keep only the 5/6/5 most significant bits:
   * red 0x0B -> 1, green 0x12 -> 4, blue 0x20 -> 4. */
  CHECK_EQ(toRgb565(RgbColor(0x0B, 0x12, 0x20)), 0x0884);
}

TEST(Rgb565RoundTripIsStableAfterFirstQuantization) {
  for (int red = 0; red <= 255; red += 17) {
    for (int green = 0; green <= 255; green += 17) {
      for (int blue = 0; blue <= 255; blue += 17) {
        const RgbColor original(static_cast<uint8_t>(red),
                                static_cast<uint8_t>(green),
                                static_cast<uint8_t>(blue));
        const RgbColor quantized = fromRgb565(toRgb565(original));
        CHECK(quantized == fromRgb565(toRgb565(quantized)));
        /* The screen drops at most 7, 3 and 7 levels per channel. */
        CHECK(static_cast<int>(original.red()) - static_cast<int>(quantized.red()) < 8);
        CHECK(static_cast<int>(original.green()) - static_cast<int>(quantized.green()) < 4);
        CHECK(static_cast<int>(original.blue()) - static_cast<int>(quantized.blue()) < 8);
      }
    }
  }
}

TEST(WithChannelClampsInsteadOfWrapping) {
  const RgbColor base(10, 20, 30);
  CHECK(withChannel(base, 0, -5) == RgbColor(0, 20, 30));
  CHECK(withChannel(base, 1, 300) == RgbColor(10, 255, 30));
  CHECK(withChannel(base, 2, 128) == RgbColor(10, 20, 128));
}

TEST(LuminanceOrdering) {
  CHECK_EQ(luminance(RgbColor(255, 255, 255)), 255);
  CHECK_EQ(luminance(RgbColor(0, 0, 0)), 0);
  CHECK(luminance(RgbColor(255, 255, 255)) > luminance(RgbColor(0, 0, 255)));
}

TEST(ChannelValuesAreValidated) {
  CHECK(RgbColor::isChannelValue(0));
  CHECK(RgbColor::isChannelValue(255));
  CHECK(!RgbColor::isChannelValue(-1));
  CHECK(!RgbColor::isChannelValue(256));
}
