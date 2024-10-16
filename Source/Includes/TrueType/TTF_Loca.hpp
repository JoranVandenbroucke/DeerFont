//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

inline auto ReadLoca(FILE *const file, const uint32 locaLocation, const uint32 nrOfGlyphs, const uint32 glyphTableLocation, const bool isShort, std::vector<uint32> &glyphLocations) noexcept
{
    if ( fseek(file, locaLocation, SEEK_SET) != 0 )
    {
        return error_code::font_parsing_error;
    }
    glyphLocations.resize(nrOfGlyphs);
    for ( auto &glyphLocation : glyphLocations )
    {
        glyphLocation = glyphTableLocation;
        if ( isShort )
        {
            uint16 offset;
            if ( const error_code errorCode = ReadValue(file, offset); errorCode != error_code::no_error )
            {
                return errorCode;
            }
            glyphLocation += offset;
        }
        else
        {
            uint32 offset;
            if ( const error_code errorCode = ReadValue(file, offset); errorCode != error_code::no_error )
            {
                return errorCode;
            }
            glyphLocation += offset;
        }
    }
    return error_code::no_error;
}
