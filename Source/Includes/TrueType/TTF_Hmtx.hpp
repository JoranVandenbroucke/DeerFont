//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"
import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

struct longHorMetric
{
    uint16 advanceWidth;
    int16  leftSideBearing;
};
struct Hmtx
{
    std::vector<longHorMetric> hMetrics; // The value numOfLongHorMetrics comes from the 'hhea' table. If the font is monospaced, only one entry need be in the array but that entry is required.
    // std::vector<FWord>         leftSideBearing; // Here the advanceWidth is assumed to be the same as the advanceWidth for the last entry above. The number of entries in this array is derived from the total number of glyphs minus
    // numOfLongHorMetrics. This generally is used with a run of monospaced glyphs (e.g. Kanji fonts or Courier fonts). Only one run is allowed and it must be at the end.
};

inline auto GetHorizontalLayoutInformation(FILE *const file, const uint32 hmtxLocation, const uint16 nrOfLongHorMetrics, const uint16 nrOfGlyphs, Hmtx &hmtx)
{
    if ( fseek(file, hmtxLocation, SEEK_SET) != 0 )
    {
        return error_code::font_parsing_error;
    }
    hmtx.hMetrics.resize(nrOfGlyphs);
    for ( uint16 i{}; i < nrOfLongHorMetrics; ++i )
    {
        if ( auto &[advanceWidth, leftSideBearing]{ hmtx.hMetrics[i] }; ReadValue(file, advanceWidth) != error_code::no_error || ReadValue(file, leftSideBearing) != error_code::no_error )
        {
            return error_code::font_parsing_error;
        }
    }
    const uint16 nrOfBarings{ static_cast<uint16>(nrOfGlyphs - nrOfLongHorMetrics) };
    const uint16 lasAdvaceWidth{ hmtx.hMetrics[nrOfLongHorMetrics - 1].advanceWidth };
    {
    }
    for ( uint16 i{}; i < nrOfBarings; ++i )
    {
        if ( ReadValue(file, hmtx.hMetrics[nrOfLongHorMetrics + i].leftSideBearing) != error_code::no_error )
        {
            return error_code::font_parsing_error;
        }
        hmtx.hMetrics[nrOfLongHorMetrics + i].advanceWidth = lasAdvaceWidth;
    }

    return error_code::no_error;
}
