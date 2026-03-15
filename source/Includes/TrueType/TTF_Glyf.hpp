//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include <algorithm>
#include <cassert>

#include "../Helpers.hpp"

import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

struct GlyphDescription
{
    int16 numberOfContours{}; // If the number of contours is positive or zero, it is a single glyph;
                              // If the number of contours less than zero, the glyph is compound
    FWord xMin{};             // Minimum x for coordinate data
    FWord yMin{};             // Minimum y for coordinate data
    FWord xMax{};             // Maximum x for coordinate data
    FWord yMax{};             // Maximum y for coordinate data
};
struct SimpleGlyphs
{
    std::vector<uint16>    endPtsOfContours;    // Array of last points of each contour; n is the number of contours; array entries are point indices
    std::vector<int16>     xCoordinates;        // Array of x-coordinates; the first is relative to (0,0), others are relative to previous point
    std::vector<int16>     yCoordinates;        // Array of y-coordinates; the first is relative to (0,0), others are relative to previous point
    std::vector<uint8>     instructions;        // Array of instructions for this glyph
    std::vector<std::byte> flags;               // Array of flags
    uint16                 instructionLength{}; // Total number of bytes needed for instructions
};
struct CompoundGlyph
{
    int16 argument1{};   // X-offset for component or point number; type depends
                         // on bits 0 and 1 in component flags
    int16 argument2{};   // Y-offset for component or point number type depends
                         // on bits 0 and 1 in component flags
    uint16 flags{};      // Component flag
    uint16 glyphIndex{}; // Glyph index of component
    FWord  xx{};
    FWord  yy{};
    FWord  xy{};
    FWord  yx{};
};
struct GlyphData
{
    std::vector<uint16>    endPtsOfControus;
    std::vector<int16>     xPositions;
    std::vector<int16>     yPositions;
    std::vector<std::byte> flags;
    GlyphDescription       description;
    uint32                 unicodeValue;
    uint32                 glyphIndex;
    uint16                 advanceWidth;
    int16                  leftSideBearing;
};
[[nodiscard]] constexpr bool isOnCurve(const std::byte flag) noexcept
{
    return (flag & std::byte{ 0x01 }) == std::byte{ 0x01 };
}

[[nodiscard]] constexpr bool isXShortVector(const std::byte flag) noexcept
{
    return (flag & std::byte{ 0x02 }) == std::byte{ 0x02 };
}

[[nodiscard]] constexpr bool isYShortVector(const std::byte flag) noexcept
{
    return (flag & std::byte{ 0x04 }) == std::byte{ 0x04 };
}

[[nodiscard]] constexpr bool isRepeat(const std::byte flag) noexcept
{
    return (flag & std::byte{ 0x08 }) == std::byte{ 0x08 };
}

[[nodiscard]] constexpr bool isXPositive(const std::byte flag) noexcept
{
    return (flag & std::byte{ 0x10 }) == std::byte{ 0x10 };
}

[[nodiscard]] constexpr bool isYPositive(const std::byte flag) noexcept
{
    return (flag & std::byte{ 0x20 }) == std::byte{ 0x20 };
}

[[nodiscard]] constexpr bool isXOrYSame(const std::byte flag, const bool isX) noexcept
{
    return (flag & (isX ? std::byte{ 0x10 } : std::byte{ 0x20 })) == (isX ? std::byte{ 0x10 } : std::byte{ 0x20 });
}

inline auto ReadGlyph(FILE *file, const std::vector<uint32> &glyphLocations, uint32 glyphIndex, uint32 numberOfGlyphs, GlyphData &glyphData) noexcept -> error_code;

inline auto ProcessRepeatFlags(FILE *const file, const uint8 flag, SimpleGlyphs &simpleGlyphs, int &i) noexcept
{
    if ( isRepeat(static_cast<std::byte>(flag)) )
    {
        uint8 repeatCount;
        if ( const error_code errorCode = ReadValue(file, repeatCount); errorCode != error_code::no_error )
            return errorCode;
        std::fill_n(simpleGlyphs.flags.begin() + i + 1, repeatCount, std::bit_cast<std::byte>(flag));
        i += repeatCount;
    }
    return error_code::no_error;
}

inline auto ProcessXCoordinates(FILE *const file, const int numberOfContourPoints, SimpleGlyphs &simpleGlyphs) noexcept
{
    int16 previousPosition = 0;
    for ( int i = 0; i < numberOfContourPoints; ++i )
    {
        if ( const std::byte flag{ simpleGlyphs.flags[i] }; isXShortVector(flag) )
        {
            uint8 position;
            if ( const error_code errorCode = ReadValue(file, position); errorCode != error_code::no_error )
            {
                return errorCode;
            }
            previousPosition += isXPositive(flag) ? position : -position;
        }
        else if ( !isXOrYSame(flag, true) )
        {
            int16 position;
            if ( const error_code errorCode = ReadValue(file, position); errorCode != error_code::no_error )
            {
                return errorCode;
            }
            previousPosition += position;
        }
        simpleGlyphs.xCoordinates[i] = previousPosition;
    }
    return error_code::no_error;
}

inline auto ProcessYCoordinates(FILE *const file, const int numberOfContourPoints, SimpleGlyphs &simpleGlyphs) noexcept
{
    int16 previousPosition = 0;

    for ( int i = 0; i < numberOfContourPoints; ++i )
    {

        if ( const std::byte flag{ simpleGlyphs.flags[i] }; isYShortVector(flag) )
        {
            uint8 position;
            if ( const error_code errorCode = ReadValue(file, position); errorCode != error_code::no_error )
            {
                return errorCode;
            }
            previousPosition += isYPositive(flag) ? position : -position;
        }
        else if ( !isXOrYSame(flag, false) )
        {
            int16 position;
            if ( const error_code errorCode = ReadValue(file, position); errorCode != error_code::no_error )
            {
                return errorCode;
            }
            previousPosition += position;
        }

        simpleGlyphs.yCoordinates[i] = previousPosition;
    }
    return error_code::no_error;
}

inline auto ReadSimpleGlyph(FILE *const file, const uint32 numberOfContours, GlyphData &glyphData) noexcept
{
    SimpleGlyphs simpleGlyphs{};
    simpleGlyphs.endPtsOfContours.resize(numberOfContours);

    uint16 numberOfContourPoints{};
    for ( auto &endPts : simpleGlyphs.endPtsOfContours )
    {
        if ( const error_code errorCode = ReadValue(file, endPts); errorCode != error_code::no_error )
        {
            return errorCode;
        }
        if ( numberOfContourPoints >= endPts )
        {
            return error_code::invalid_font_format;
        }

        numberOfContourPoints = endPts;
    }
    ++numberOfContourPoints;

    if ( const error_code errorCode = ReadValue(file, simpleGlyphs.instructionLength); errorCode != error_code::no_error )
    {
        return errorCode;
    }
    if ( std::fseek(file, simpleGlyphs.instructionLength, SEEK_CUR) != 0 )
    {
        return error_code::font_parsing_error;
    }


    simpleGlyphs.flags.resize(numberOfContourPoints);
    for ( int i = 0; i < numberOfContourPoints; ++i )
    {
        uint8 flag;
        if ( const error_code errorCode = ReadValue(file, flag); errorCode != error_code::no_error )
        {
            return errorCode;
        }
        simpleGlyphs.flags[i] = std::bit_cast<std::byte>(flag);

        if ( const error_code errorCode = ProcessRepeatFlags(file, flag, simpleGlyphs, i); errorCode != error_code::no_error )
        {
            return errorCode;
        }
    }

    simpleGlyphs.xCoordinates.resize(numberOfContourPoints);
    if ( const error_code errorCode = ProcessXCoordinates(file, numberOfContourPoints, simpleGlyphs); errorCode != error_code::no_error )
    {
        return errorCode;
    }

    simpleGlyphs.yCoordinates.resize(numberOfContourPoints);
    if ( const error_code errorCode = ProcessYCoordinates(file, numberOfContourPoints, simpleGlyphs); errorCode != error_code::no_error )
    {
        return errorCode;
    }
    glyphData.xPositions.insert(glyphData.xPositions.end(), simpleGlyphs.xCoordinates.cbegin(), simpleGlyphs.xCoordinates.cend());
    glyphData.yPositions.insert(glyphData.yPositions.end(), simpleGlyphs.yCoordinates.cbegin(), simpleGlyphs.yCoordinates.cend());
    glyphData.flags.insert(glyphData.flags.end(), simpleGlyphs.flags.cbegin(), simpleGlyphs.flags.cend());
    glyphData.endPtsOfControus.insert(glyphData.endPtsOfControus.end(), simpleGlyphs.endPtsOfContours.cbegin(), simpleGlyphs.endPtsOfContours.cend());
    return error_code::no_error;
}

inline auto ReadArguments(FILE *const file, const bool argAreXY, const bool argAreWords, CompoundGlyph &compoundGlyph) noexcept
{
    using enum error_code;

    if ( argAreXY )
    {
        error_code errorCode = argAreWords ? ReadValue(file, compoundGlyph.argument1) : ReadValue<int8>(file, compoundGlyph.argument1);
        if ( errorCode != no_error )
        {
            return errorCode;
        }
        errorCode = argAreWords ? ReadValue(file, compoundGlyph.argument2) : ReadValue<int8>(file, compoundGlyph.argument2);
        if ( errorCode != no_error )
        {
            return errorCode;
        }
    }
    else
    {
        error_code errorCode = argAreWords ? ReadValue<uint16>(file, compoundGlyph.argument1) : ReadValue<uint8>(file, compoundGlyph.argument1);
        if ( errorCode != no_error )
        {
            return errorCode;
        }
        errorCode = argAreWords ? ReadValue<uint16>(file, compoundGlyph.argument2) : ReadValue<uint8>(file, compoundGlyph.argument2);
        if ( errorCode != no_error )
        {
            return errorCode;
        }
    }

    return no_error;
}

inline auto ApplyScale(FILE *const file, const std::vector<uint32> &glyphLocations, const uint32 nrOfGlyphs, const bool hasScale, const bool hasXYScale, const bool hasTwoByTwo, CompoundGlyph &compoundGlyph, GlyphData &glyphData) noexcept
{
    using enum error_code;
    compoundGlyph.xx = static_cast<FWord>(0x10000);
    compoundGlyph.yy = static_cast<FWord>(0x10000);
    compoundGlyph.xy = 0;
    compoundGlyph.yx = 0;

    if ( hasScale )
    {
        if ( ReadValue(file, compoundGlyph.xx) != no_error )
        {
            return font_parsing_error;
        }
        compoundGlyph.xx *= 4;
        compoundGlyph.yy = compoundGlyph.xx;
    }
    else if ( hasXYScale )
    {
        if ( ReadValue(file, compoundGlyph.xx) != no_error || ReadValue(file, compoundGlyph.yy) != no_error )
        {
            return font_parsing_error;
        }
        compoundGlyph.xx *= 4;
        compoundGlyph.yy *= 4;
    }
    else if ( hasTwoByTwo )
    {
        if ( ReadValue(file, compoundGlyph.xx) != no_error || ReadValue(file, compoundGlyph.xy) != no_error || ReadValue(file, compoundGlyph.yx) != no_error || ReadValue(file, compoundGlyph.yy) != no_error )
        {
            return font_parsing_error;
        }
        compoundGlyph.xx *= 4;
        compoundGlyph.xy *= 4;
        compoundGlyph.yx *= 4;
        compoundGlyph.yy *= 4;
    }

    const uint32 currentPosition{ static_cast<uint32>(ftell(file)) };
    GlyphData    simpleGlyph;
    if ( const error_code errorCode = ReadGlyph(file, glyphLocations, compoundGlyph.glyphIndex, nrOfGlyphs, simpleGlyph); errorCode != no_error )
    {
        return errorCode;
    }
    fseek(file, currentPosition, SEEK_SET);

    for ( int i = 0; i < simpleGlyph.xPositions.size(); i++ )
    {
        simpleGlyph.xPositions[i] = compoundGlyph.xx * simpleGlyph.xPositions[i] + compoundGlyph.xy * simpleGlyph.yPositions[i] + compoundGlyph.argument1;
        simpleGlyph.yPositions[i] = compoundGlyph.yx * simpleGlyph.xPositions[i] + compoundGlyph.yy * simpleGlyph.yPositions[i] + compoundGlyph.argument2;
    }

    glyphData.xPositions.insert(glyphData.xPositions.end(), simpleGlyph.xPositions.cbegin(), simpleGlyph.xPositions.cend());
    glyphData.yPositions.insert(glyphData.yPositions.end(), simpleGlyph.yPositions.cbegin(), simpleGlyph.yPositions.cend());
    glyphData.flags.insert(glyphData.flags.end(), simpleGlyph.flags.cbegin(), simpleGlyph.flags.cend());
    glyphData.endPtsOfControus.insert(glyphData.endPtsOfControus.end(), simpleGlyph.endPtsOfControus.cbegin(), simpleGlyph.endPtsOfControus.cend());
    return no_error;
}

inline auto ReadCompoundGlyph(FILE *const file, const std::vector<uint32> &glyphLocations, const uint32 numberOfGlyphs, GlyphData &glyphData) noexcept
{
    using enum error_code;
    bool MORE_COMPONENTS{};
    do
    {
        CompoundGlyph compoundGlyph{};
        if ( ReadValue(file, compoundGlyph.flags) != no_error || ReadValue(file, compoundGlyph.glyphIndex) != no_error )
        {
            return font_parsing_error;
        }

        // ignore invalid glyph number
        if ( compoundGlyph.glyphIndex >= numberOfGlyphs )
        {
            return invalid_font_format;
        }

        const bool ARG_1_AND_2_ARE_WORDS{ (compoundGlyph.flags & 0x001) != 0 };    // If set, the arguments are words;
                                                                                   // If not set, they are bytes.
        const bool ARGS_ARE_XY_VALUES{ (compoundGlyph.flags & 0x002) != 0 };       // If set, the arguments are xy values;
                                                                                   // If not set, they are points.
        const bool WE_HAVE_A_SCALE{ (compoundGlyph.flags & 0x008) != 0 };          // If set, there is a simple scale for the component.
                                                                                   // If not set, scale is 1.0.
        MORE_COMPONENTS = (compoundGlyph.flags & 0x020) != 0;                      // If set, at least one additional glyph follows this one.
        const bool WE_HAVE_AN_X_AND_Y_SCALE{ (compoundGlyph.flags & 0x040) != 0 }; // If set the x direction will use a different scale than the y direction.
        const bool WE_HAVE_A_TWO_BY_TWO{ (compoundGlyph.flags & 0x080) != 0 };     // If set there is a 2-by-2 transformation that will be used to scale the component.

        if ( const error_code errorCode = ReadArguments(file, ARGS_ARE_XY_VALUES, ARG_1_AND_2_ARE_WORDS, compoundGlyph); errorCode != no_error )
        {
            return errorCode;
        }
        if ( const error_code errorCode = ApplyScale(file, glyphLocations, numberOfGlyphs, WE_HAVE_A_SCALE, WE_HAVE_AN_X_AND_Y_SCALE, WE_HAVE_A_TWO_BY_TWO, compoundGlyph, glyphData); errorCode != no_error )
        {
            return errorCode;
        }

    } while ( MORE_COMPONENTS );
    return no_error;
}

inline auto ReadGlyph(FILE *const file, const std::vector<uint32> &glyphLocations, const uint32 glyphIndex, const uint32 numberOfGlyphs, GlyphData &glyphData) noexcept -> error_code
{
    using enum error_code;
    if ( std::fseek(file, glyphLocations[glyphIndex], SEEK_SET) != 0 )
    {
        return font_parsing_error;
    }

    if ( ReadValue(file, glyphData.description.numberOfContours) != no_error || ReadValue(file, glyphData.description.xMin) != no_error || ReadValue(file, glyphData.description.yMin) != no_error ||
         ReadValue(file, glyphData.description.xMax) != no_error || ReadValue(file, glyphData.description.yMax) != no_error )
    {
        return font_parsing_error;
    }

    if ( glyphData.description.numberOfContours > 0 )
    {
        return ReadSimpleGlyph(file, glyphData.description.numberOfContours, glyphData);
    }
    if ( glyphData.description.numberOfContours < 0 )
    {
        return ReadCompoundGlyph(file, glyphLocations, numberOfGlyphs, glyphData);
    }

    return invalid_font_format;
}
inline auto ReadAllGlyphs(FILE *const file, const std::vector<uint32> &glyphLocations, const std::vector<std::pair<uint32, uint32>> &glyphMappings, std::vector<GlyphData> &glyphDatas)
{
    const uint32 glyphLocationsSize{static_cast<uint32>(glyphLocations.size())};
    const std::size_t glyphMappingsSize{glyphMappings.size()};
    glyphDatas.resize(glyphMappingsSize);

    for ( int i = 0; i < glyphMappingsSize; ++i )
    {
        const auto &[index, code]{ glyphMappings[i] };
        if ( const error_code errorCode = ReadGlyph(file, glyphLocations, index, glyphLocationsSize, glyphDatas[i]); errorCode != error_code::no_error )
        {
            return errorCode;
        }
        glyphDatas[i].glyphIndex   = index;
        glyphDatas[i].unicodeValue = code;
    }
    return error_code::no_error;
}
