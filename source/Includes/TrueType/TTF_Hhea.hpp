//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"
import FawnAlgebra;
import std;
using namespace fawn_algebra;

struct TtfHhea
{
    Fixed version{};                     // 0x00010000 (1.0)
    FWord ascent{};                      // Distance from baseline of highest ascender
    FWord descent{};                     // Distance from baseline of lowest descender
    FWord lineGap{};                     // typographic line gap
    uFWord advanceWidthMax{};            // must be consistent with horizontal metrics
    FWord minLeftSideBearing{};          // must be consistent with horizontal metrics
    FWord minRightSideBearing{};         // must be consistent with horizontal metrics
    FWord xMaxExtent{};                  // max(lsb + (xMax-xMin))
    std::int16_t caretSlopeRise{};       // used to calculate the slope of the caret (rise/run) set to 1 for vertical caret
    std::int16_t caretSlopeRun{};        // 0 for vertical
    FWord caretOffset{};                 // set value to 0 for non-slanted fonts
    std::int64_t reserved{};             // set value to 0
    std::int16_t metricDataFormat{};     // 0 for current format
    std::uint16_t numOfLongHorMetrics{}; // number of advance widths in metrics table
};
inline auto GetNumAdvanceWidthMetrics(std::FILE* pFile, const std::uint32_t hheaLocation, std::uint16_t& numOfLongHorMetrics)
{
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, hheaLocation, 0) != 0)
    {
        return error_code::font_parsing_error;
    }

    TtfHhea hhea;
    if (ReadValue(pFile, hhea.version) != error_code::no_error || ReadValue(pFile, hhea.ascent) != error_code::no_error || ReadValue(pFile, hhea.descent) != error_code::no_error || ReadValue(pFile, hhea.lineGap) != error_code::no_error
        || ReadValue(pFile, hhea.advanceWidthMax) != error_code::no_error || ReadValue(pFile, hhea.minLeftSideBearing) != error_code::no_error || ReadValue(pFile, hhea.minRightSideBearing) != error_code::no_error
        || ReadValue(pFile, hhea.xMaxExtent) != error_code::no_error || ReadValue(pFile, hhea.caretSlopeRise) != error_code::no_error || ReadValue(pFile, hhea.caretSlopeRun) != error_code::no_error
        || ReadValue(pFile, hhea.caretOffset) != error_code::no_error || ReadValue(pFile, hhea.reserved) != error_code::no_error || ReadValue(pFile, hhea.metricDataFormat) != error_code::no_error
        || ReadValue(pFile, hhea.numOfLongHorMetrics) != error_code::no_error)
    {
        return error_code::font_parsing_error;
    }

    numOfLongHorMetrics = hhea.numOfLongHorMetrics;
    return error_code::no_error;
}
