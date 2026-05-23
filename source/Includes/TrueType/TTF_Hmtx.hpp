//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

struct LongHorMetric
{
    std::uint16_t advanceWidth{};
    std::int16_t leftSideBearing{};
};
struct Hmtx
{
    std::vector<LongHorMetric> hMetrics; // The value numOfLongHorMetrics comes from the 'hhea' table. If the font is monospaced, only one entry need be in the array but that entry is required.
    // std::vector<FWord>         leftSideBearing; // Here the advanceWidth is assumed to be the same as the advanceWidth for the last entry above. The number of entries in this array is derived from the total number of glyphs minus
    // numOfLongHorMetrics. This generally is used with a run of monospaced glyphs (e.g. Kanji fonts or Courier fonts). Only one run is allowed and it must be at the end.
};

inline auto GetHorizontalLayoutInformation(std::FILE* const pFile, const std::uint32_t hmtxLocation, const std::uint16_t nrOfLongHorMetrics, const std::uint16_t nrOfGlyphs, Hmtx& hmtx)
{
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, hmtxLocation, 0) != 0)
    {
        return error_code::font_parsing_error;
    }
    hmtx.hMetrics.resize(nrOfGlyphs);
    for (std::uint16_t i{}; i < nrOfLongHorMetrics; ++i)
    {
        if (auto& [advanceWidth, leftSideBearing]{hmtx.hMetrics[i]}; ReadValue(pFile, advanceWidth) != error_code::no_error || ReadValue(pFile, leftSideBearing) != error_code::no_error)
        {
            return error_code::font_parsing_error;
        }
    }
    const std::uint16_t nrOfBarings{static_cast<std::uint16_t>(nrOfGlyphs - nrOfLongHorMetrics)};
    const std::uint16_t lasAdvaceWidth{hmtx.hMetrics[nrOfLongHorMetrics - 1].advanceWidth};
    {
    }
    for (std::uint16_t i{}; i < nrOfBarings; ++i)
    {
        if (ReadValue(pFile, hmtx.hMetrics[nrOfLongHorMetrics + i].leftSideBearing) != error_code::no_error)
        {
            return error_code::font_parsing_error;
        }
        hmtx.hMetrics[nrOfLongHorMetrics + i].advanceWidth = lasAdvaceWidth;
    }

    return error_code::no_error;
}
