//
// Copyright (c) 2024.
// Author: Joran.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>

import DeerFont;
using namespace DeerFont;

TEST_CASE("ReadFont: Valid font file", "[TrueType]")
{
    Font           font;
    const char8_t *filePath{ u8"fonts/JetBrainsMono-Bold.ttf" };
    const auto     result{ ReadFont(filePath, font) };

    REQUIRE(result == 0);
    REQUIRE_FALSE(font.glyphs.empty());
}

TEST_CASE("ReadFont: Invalid font file", "[TrueType]")
{
    Font           font;
    const char8_t *filePath{ u8"fonts/InvalidFont.ttf" };
    const int      result{ ReadFont(filePath, font) };

    REQUIRE(result == -1);
}

TEST_CASE("ReadFont: Font with no glyphs", "[TrueType]")
{
    Font           font;
    // todo : add empty font
    const char8_t *filePath{ u8"fonts/EmptyFont.ttf" };
    const int      result{ ReadFont(filePath, font) };

    REQUIRE(result == -1);
}
