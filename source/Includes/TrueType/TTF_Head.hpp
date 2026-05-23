//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

struct Head
{
    Fixed version{};                    // 0x00010000 if (version 1.0)
    Fixed fontRevision{};               // set by font manufacturer
    std::uint32_t checkSumAdjustment{}; // To compute: set it to 0, calculate the checksum for the 'head' table and put it in the table directory, sum the entire font as a std::uint32_t, then store 0xB1B0AFBA - sum. (The checksum for the
                                        // 'head' table will be wrong as a result. That is OK; do not reset it.)
    std::uint32_t magicNumber{};        // set to 0x5F0F3CF5
    std::uint16_t flags{};              // bit 0 - y value of 0 specifies baseline
                                        // bit 1 - x position of left most black bit is LSB
                                        // bit 2 - scaled point size and actual point size will differ (i.e. 24 point glyph differs from 12 point glyph scaled by factor of 2)
                                        // bit 3 - use integer scaling instead of fractional
                                        // bit 4 - (used by the Microsoft implementation of the TrueType scaler)
    // bit 5 - This bit should be set in fonts that are intended to e laid out vertically, and in which the glyphs have been drawn such that an x-coordinate of 0 corresponds to the desired vertical baseline.
    // bit 6 - This bit must be set to zero.
    // bit 7 - This bit should be set if the font requires layout for correct linguistic rendering (e.g. Arabic fonts).
    // bit 8 - This bit should be set for an AAT font which has one or more metamorphosis effects designated as happening by default.
    // bit 9 - This bit should be set if the font contains any strong right-to-left glyphs.
    // bit 10 - This bit should be set if the font contains Indic-style rearrangement effects.
    // bits 11-13 - Defined by Adobe.
    // bit 14 - This bit should be set if the glyphs in the font are simply generic symbols for code point ranges, such as for a last resort font.
    std::uint16_t unitsPerEm{};       // range from 64 to 16384
    longDateTime created{};           // international date
    longDateTime modified{};          // international date
    FWord xMin{};                     // for all glyph bounding boxes
    FWord yMin{};                     // for all glyph bounding boxes
    FWord xMax{};                     // for all glyph bounding boxes
    FWord yMax{};                     // for all glyph bounding boxes
    std::uint16_t macStyle{};         // bit 0 bold
                                      // bit 1 italic
                                      // bit 2 underline
                                      // bit 3 outline
                                      // bit 4 shadow
                                      // bit 5 condensed (narrow)
                                      // bit 6 extended
    std::uint16_t lowestRecPPEM{};    // smallest readable size in pixels
    std::int16_t fontDirectionHint{}; // 0 Mixed directional glyphs
                                      // 1 Only strongly left to right glyphs
                                      // 2 Like 1 but also contains neutrals
                                      // -1 Only strongly right to left glyphs
                                      // -2 Like -1 but also contains neutrals
    std::int16_t indexToLocFormat{};  // 0 for short offsets, 1 for long
    std::int16_t glyphDataFormat{};   // 0 for current format
};

inline auto ReadHeader(std::FILE* const pFile, const std::uint32_t headLocation, Head& header) noexcept
{
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, headLocation, 0) != 0)
    {
        return error_code::font_parsing_error;
    }
    if (ReadValue(pFile, header.version) != error_code::no_error || ReadValue(pFile, header.fontRevision) != error_code::no_error || ReadValue(pFile, header.checkSumAdjustment) != error_code::no_error
        || ReadValue(pFile, header.magicNumber) != error_code::no_error || ReadValue(pFile, header.flags) != error_code::no_error || ReadValue(pFile, header.unitsPerEm) != error_code::no_error
        || ReadValue(pFile, header.created) != error_code::no_error || ReadValue(pFile, header.modified) != error_code::no_error || ReadValue(pFile, header.xMin) != error_code::no_error || ReadValue(pFile, header.yMin) != error_code::no_error
        || ReadValue(pFile, header.xMax) != error_code::no_error || ReadValue(pFile, header.yMax) != error_code::no_error || ReadValue(pFile, header.macStyle) != error_code::no_error
        || ReadValue(pFile, header.lowestRecPPEM) != error_code::no_error || ReadValue(pFile, header.fontDirectionHint) != error_code::no_error || ReadValue(pFile, header.indexToLocFormat) != error_code::no_error
        || ReadValue(pFile, header.glyphDataFormat) != error_code::no_error)
    {
        return error_code::font_parsing_error;
    }
    return error_code::no_error;
}
