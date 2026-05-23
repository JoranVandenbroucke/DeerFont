//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

struct Maxp
{
    Fixed version{};           // 0x00005000 (0.5)
    std::uint16_t numGlyphs{}; // the number of glyphs in the font
};

inline auto ReadMaxp(std::FILE* const pFile, const std::uint32_t mapxLocation, Maxp& maxp) noexcept
{
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, mapxLocation, 0) != 0)
    {
        return error_code::font_parsing_error;
    }
    if (ReadValue(pFile, maxp.version) != error_code::no_error || ReadValue(pFile, maxp.numGlyphs) != error_code::no_error)
    {
        return error_code::font_parsing_error;
    }
    return error_code::no_error;
}
