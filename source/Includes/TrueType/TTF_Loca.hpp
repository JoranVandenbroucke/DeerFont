#ifndef HOME_JORAN_DEV_BALBINO_SOURCE_ENGINE_RENDERER_FONT_SOURCE_INCLUDES_TRUETYPE_TTF_LOCA_HPP
#define HOME_JORAN_DEV_BALBINO_SOURCE_ENGINE_RENDERER_FONT_SOURCE_INCLUDES_TRUETYPE_TTF_LOCA_HPP

//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

inline auto ReadLoca(std::FILE* const pFile, const std::uint32_t locaLocation, const std::uint32_t nrOfGlyphs, const std::uint32_t glyphTableLocation, const bool isShort, std::vector<std::uint32_t>& glyphLocations) noexcept
{
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, locaLocation, 0) != 0)
    {
        return error_code::font_parsing_error;
    }
    glyphLocations.resize(nrOfGlyphs);
    for (auto& glyphLocation : glyphLocations)
    {
        glyphLocation = glyphTableLocation;
        if (isShort)
        {
            std::uint16_t offset;
            if (const error_code errorCode = ReadValue(pFile, offset); errorCode != error_code::no_error)
            {
                return errorCode;
            }
            glyphLocation += offset;
        }
        else
        {
            std::uint32_t offset;
            if (const error_code errorCode = ReadValue(pFile, offset); errorCode != error_code::no_error)
            {
                return errorCode;
            }
            glyphLocation += offset;
        }
    }
    return error_code::no_error;
}

#endif
