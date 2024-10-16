//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

struct Maxp
{
    Fixed    version;   // 0x00005000 (0.5)
    uint16 numGlyphs; // the number of glyphs in the font
};

inline auto ReadMaxp(FILE* const file, const uint32 mapxLocation, Maxp& maxp) noexcept
{
    if(fseek(file, mapxLocation, SEEK_SET)!=0)
    {
        return error_code::font_parsing_error;
    }
    if(ReadValue(file, maxp.version) != error_code::no_error || ReadValue(file, maxp.numGlyphs) != error_code::no_error)
    {
        return error_code::font_parsing_error;
    }
    return error_code::no_error;
}