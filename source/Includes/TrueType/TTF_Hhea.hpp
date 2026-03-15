//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"
import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

struct TTF_Hhea
{
    Fixed  version{};             // 0x00010000 (1.0)
    FWord  ascent{};              // Distance from baseline of highest ascender
    FWord  descent{};             // Distance from baseline of lowest descender
    FWord  lineGap{};             // typographic line gap
    uFWord advanceWidthMax{};     // must be consistent with horizontal metrics
    FWord  minLeftSideBearing{};  // must be consistent with horizontal metrics
    FWord  minRightSideBearing{}; // must be consistent with horizontal metrics
    FWord  xMaxExtent{};          // max(lsb + (xMax-xMin))
    int16  caretSlopeRise{};      // used to calculate the slope of the caret (rise/run) set to 1 for vertical caret
    int16  caretSlopeRun{};       // 0 for vertical
    FWord  caretOffset{};         // set value to 0 for non-slanted fonts
    int64  reserved{};            // set value to 0
    int16  metricDataFormat{};    // 0 for current format
    uint16 numOfLongHorMetrics{}; // number of advance widths in metrics table
};
inline auto GetNumAdvanceWidthMetrics(FILE *file, const uint32 hheaLocation, uint16 &numOfLongHorMetrics)
{
    if ( fseek(file, hheaLocation, SEEK_SET) != 0 )
    {
        return error_code::font_parsing_error;
    }

    TTF_Hhea hhea;
    if ( ReadValue(file, hhea.version) != error_code::no_error || ReadValue(file, hhea.ascent) != error_code::no_error || ReadValue(file, hhea.descent) != error_code::no_error || ReadValue(file, hhea.lineGap) != error_code::no_error ||
         ReadValue(file, hhea.advanceWidthMax) != error_code::no_error || ReadValue(file, hhea.minLeftSideBearing) != error_code::no_error || ReadValue(file, hhea.minRightSideBearing) != error_code::no_error ||
         ReadValue(file, hhea.xMaxExtent) != error_code::no_error || ReadValue(file, hhea.caretSlopeRise) != error_code::no_error || ReadValue(file, hhea.caretSlopeRun) != error_code::no_error ||
         ReadValue(file, hhea.caretOffset) != error_code::no_error || ReadValue(file, hhea.reserved) != error_code::no_error || ReadValue(file, hhea.metricDataFormat) != error_code::no_error ||
         ReadValue(file, hhea.numOfLongHorMetrics) != error_code::no_error )
    {
        return error_code::font_parsing_error;
    }

    numOfLongHorMetrics = hhea.numOfLongHorMetrics;
    return error_code::no_error;
}
