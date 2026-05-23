//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

struct GlyphDescription
{
    std::int16_t numberOfContours{}; // If the number of contours is positive or zero, it is a single glyph;
                                     // If the number of contours less than zero, the glyph is compound
    FWord xMin{};                    // Minimum x for coordinate data
    FWord yMin{};                    // Minimum y for coordinate data
    FWord xMax{};                    // Maximum x for coordinate data
    FWord yMax{};                    // Maximum y for coordinate data
};
struct ImpleGlyphs
{
    std::vector<std::uint16_t> endPtsOfContours{}; // Array of last points of each contour; n is the number of contours; array entries are point indices
    std::vector<std::int16_t> xCoordinates{};      // Array of x-coordinates; the first is relative to (0,0), others are relative to previous point
    std::vector<std::int16_t> yCoordinates{};      // Array of y-coordinates; the first is relative to (0,0), others are relative to previous point
    std::vector<std::uint8_t> instructions{};      // Array of instructions for this glyph
    std::vector<std::byte> flags{};                // Array of flags
    std::uint16_t instructionLength{};             // Total number of bytes needed for instructions
};
struct CompoundGlyph
{
    std::int16_t argument1{};   // X-offset for component or point number; type depends
                                // on bits 0 and 1 in component flags
    std::int16_t argument2{};   // Y-offset for component or point number type depends
                                // on bits 0 and 1 in component flags
    std::uint16_t flags{};      // Component flag
    std::uint16_t glyphIndex{}; // Glyph index of component
    FWord xx{};
    FWord yy{};
    FWord xy{};
    FWord yx{};
};
struct GlyphData
{
    std::vector<std::uint16_t> endPtsOfControus{};
    std::vector<std::int16_t> xPositions{};
    std::vector<std::int16_t> yPositions{};
    std::vector<std::byte> flags{};
    GlyphDescription description{};
    std::uint32_t unicodeValue{};
    std::uint32_t glyphIndex{};
    std::uint16_t advanceWidth{};
    std::int16_t leftSideBearing{};
};
[[nodiscard]] constexpr auto IsOnCurve(const std::byte flag) noexcept -> bool
{
    return (flag & std::byte{0x01}) == std::byte{0x01};
}

[[nodiscard]] constexpr auto IsXShortVector(const std::byte flag) noexcept -> bool
{
    return (flag & std::byte{0x02}) == std::byte{0x02};
}

[[nodiscard]] constexpr auto IsYShortVector(const std::byte flag) noexcept -> bool
{
    return (flag & std::byte{0x04}) == std::byte{0x04};
}

[[nodiscard]] constexpr auto IsRepeat(const std::byte flag) noexcept -> bool
{
    return (flag & std::byte{0x08}) == std::byte{0x08};
}

[[nodiscard]] constexpr auto IsXPositive(const std::byte flag) noexcept -> bool
{
    return (flag & std::byte{0x10}) == std::byte{0x10};
}

[[nodiscard]] constexpr auto IsYPositive(const std::byte flag) noexcept -> bool
{
    return (flag & std::byte{0x20}) == std::byte{0x20};
}

[[nodiscard]] constexpr auto IsXOrYSame(const std::byte flag, const bool isX) noexcept -> bool
{
    return (flag & (isX ? std::byte{0x10} : std::byte{0x20})) == (isX ? std::byte{0x10} : std::byte{0x20});
}

inline auto ReadGlyph(std::FILE* pFile, const std::vector<std::uint32_t>& glyphLocations, std::uint32_t glyphIndex, std::uint32_t numberOfGlyphs, GlyphData& glyphData) noexcept -> error_code;

inline auto ProcessRepeatFlags(std::FILE* const pFile, const std::uint8_t flag, ImpleGlyphs& simpleGlyphs, std::uint16_t& idx) noexcept
{
    if (IsRepeat(static_cast<std::byte>(flag)))
    {
        std::uint8_t repeatCount{};
        if (const error_code errorCode{ReadValue(pFile, repeatCount)}; errorCode != error_code::no_error)
        {
            return errorCode;
        }

        std::fill_n(simpleGlyphs.flags.begin() + idx + 1, repeatCount, std::bit_cast<std::byte>(flag));
        idx += repeatCount;
    }

    return error_code::no_error;
}

inline auto ProcessXCoordinates(std::FILE* const pFile, const std::uint16_t numberOfContourPoints, ImpleGlyphs& simpleGlyphs) noexcept
{
    std::int16_t previousPosition{};
    for (std::uint16_t idx{}; idx < numberOfContourPoints; ++idx)
    {
        if (const std::byte flag{simpleGlyphs.flags[idx]}; IsXShortVector(flag))
        {
            std::uint8_t position{};
            if (const error_code errorCode{ReadValue(pFile, position)}; errorCode != error_code::no_error)
            {
                return errorCode;
            }
            previousPosition = static_cast<std::int16_t>(previousPosition + IsXPositive(flag) ? position : -position);
        }
        else if (!IsXOrYSame(flag, true))
        {
            std::int16_t position{};
            if (const error_code errorCode{ReadValue(pFile, position)}; errorCode != error_code::no_error)
            {
                return errorCode;
            }
            previousPosition += position;
        }
        simpleGlyphs.xCoordinates[idx] = previousPosition;
    }
    return error_code::no_error;
}

inline auto ProcessYCoordinates(std::FILE* const pFile, const std::uint16_t numberOfContourPoints, ImpleGlyphs& simpleGlyphs) noexcept
{
    std::int16_t previousPosition{};

    for (std::uint16_t idx{}; idx < numberOfContourPoints; ++idx)
    {
        if (const std::byte flag{simpleGlyphs.flags[idx]}; IsYShortVector(flag))
        {
            std::uint8_t position{};
            if (const error_code errorCode{ReadValue(pFile, position)}; errorCode != error_code::no_error)
            {
                return errorCode;
            }
            previousPosition = static_cast<std::int16_t>(previousPosition + IsYPositive(flag) ? position : -position);
        }
        else if (!IsXOrYSame(flag, false))
        {
            std::int16_t position{};
            if (const error_code errorCode{ReadValue(pFile, position)}; errorCode != error_code::no_error)
            {
                return errorCode;
            }
            previousPosition += position;
        }

        simpleGlyphs.yCoordinates[idx] = previousPosition;
    }
    return error_code::no_error;
}

inline auto ReadSimpleGlyph(std::FILE* const pFile, const std::int16_t numberOfContours, GlyphData& glyphData) noexcept
{
    ImpleGlyphs simpleGlyphs{};
    simpleGlyphs.endPtsOfContours.resize(static_cast<std::size_t>(numberOfContours));

    std::uint16_t numberOfContourPoints{};
    for (auto& endPts : simpleGlyphs.endPtsOfContours)
    {
        if (const error_code errorCode{ReadValue(pFile, endPts)}; errorCode != error_code::no_error)
        {
            return errorCode;
        }
        if (numberOfContourPoints >= endPts)
        {
            return error_code::invalid_font_format;
        }

        numberOfContourPoints = endPts;
    }
    ++numberOfContourPoints;

    if (const error_code errorCode{ReadValue(pFile, simpleGlyphs.instructionLength)}; errorCode != error_code::no_error)
    {
        return errorCode;
    }
    // todo : 1 == SEEK_CUR, make sure it's no longer hard coded
    if (std::fseek(pFile, simpleGlyphs.instructionLength, 0) != 0)
    {
        return error_code::font_parsing_error;
    }

    simpleGlyphs.flags.resize(numberOfContourPoints);
    for (std::uint16_t idx{}; idx < numberOfContourPoints; ++idx)
    {
        std::uint8_t flag{};
        if (const error_code errorCode{ReadValue(pFile, flag)}; errorCode != error_code::no_error)
        {
            return errorCode;
        }
        simpleGlyphs.flags[idx] = std::bit_cast<std::byte>(flag);

        if (const error_code errorCode{ProcessRepeatFlags(pFile, flag, simpleGlyphs, idx)}; errorCode != error_code::no_error)
        {
            return errorCode;
        }
    }

    simpleGlyphs.xCoordinates.resize(numberOfContourPoints);
    if (const error_code errorCode{ProcessXCoordinates(pFile, numberOfContourPoints, simpleGlyphs)}; errorCode != error_code::no_error)
    {
        return errorCode;
    }

    simpleGlyphs.yCoordinates.resize(numberOfContourPoints);
    if (const error_code errorCode{ProcessYCoordinates(pFile, numberOfContourPoints, simpleGlyphs)}; errorCode != error_code::no_error)
    {
        return errorCode;
    }
    glyphData.xPositions.insert(glyphData.xPositions.end(), simpleGlyphs.xCoordinates.cbegin(), simpleGlyphs.xCoordinates.cend());
    glyphData.yPositions.insert(glyphData.yPositions.end(), simpleGlyphs.yCoordinates.cbegin(), simpleGlyphs.yCoordinates.cend());
    glyphData.flags.insert(glyphData.flags.end(), simpleGlyphs.flags.cbegin(), simpleGlyphs.flags.cend());
    glyphData.endPtsOfControus.insert(glyphData.endPtsOfControus.end(), simpleGlyphs.endPtsOfContours.cbegin(), simpleGlyphs.endPtsOfContours.cend());
    return error_code::no_error;
}

inline auto ReadArguments(std::FILE* const pFile, const bool argAreXY, const bool argAreWords, CompoundGlyph& compoundGlyph) noexcept
{
    using enum error_code;

    if (argAreXY)
    {
        error_code errorCode{argAreWords ? ReadValue(pFile, compoundGlyph.argument1) : ReadValue<std::int8_t>(pFile, compoundGlyph.argument1)};
        if (errorCode != no_error)
        {
            return errorCode;
        }
        errorCode = argAreWords ? ReadValue(pFile, compoundGlyph.argument2) : ReadValue<std::int8_t>(pFile, compoundGlyph.argument2);
        if (errorCode != no_error)
        {
            return errorCode;
        }
    }
    else
    {
        error_code errorCode{argAreWords ? ReadValue<std::uint16_t>(pFile, compoundGlyph.argument1) : ReadValue<std::uint8_t>(pFile, compoundGlyph.argument1)};
        if (errorCode != no_error)
        {
            return errorCode;
        }
        errorCode = argAreWords ? ReadValue<std::uint16_t>(pFile, compoundGlyph.argument2) : ReadValue<std::uint8_t>(pFile, compoundGlyph.argument2);
        if (errorCode != no_error)
        {
            return errorCode;
        }
    }

    return no_error;
}

inline auto ApplyScale(std::FILE* const pFile, const std::vector<std::uint32_t>& glyphLocations, const std::uint32_t nrOfGlyphs, const bool hasScale, const bool hasXYScale, const bool hasTwoByTwo, CompoundGlyph& compoundGlyph,
                       GlyphData& glyphData) noexcept
{
    using enum error_code;
    compoundGlyph.xx = static_cast<FWord>(0x10000);
    compoundGlyph.yy = static_cast<FWord>(0x10000);
    compoundGlyph.xy = 0;
    compoundGlyph.yx = 0;

    if (hasScale)
    {
        if (ReadValue(pFile, compoundGlyph.xx) != no_error)
        {
            return font_parsing_error;
        }
        compoundGlyph.xx *= 4;
        compoundGlyph.yy  = compoundGlyph.xx;
    }
    else if (hasXYScale)
    {
        if (ReadValue(pFile, compoundGlyph.xx) != no_error || ReadValue(pFile, compoundGlyph.yy) != no_error)
        {
            return font_parsing_error;
        }
        compoundGlyph.xx *= 4;
        compoundGlyph.yy *= 4;
    }
    else if (hasTwoByTwo)
    {
        if (ReadValue(pFile, compoundGlyph.xx) != no_error || ReadValue(pFile, compoundGlyph.xy) != no_error || ReadValue(pFile, compoundGlyph.yx) != no_error || ReadValue(pFile, compoundGlyph.yy) != no_error)
        {
            return font_parsing_error;
        }
        compoundGlyph.xx *= 4;
        compoundGlyph.xy *= 4;
        compoundGlyph.yx *= 4;
        compoundGlyph.yy *= 4;
    }

    const std::uint32_t currentPosition{static_cast<std::uint32_t>(std::ftell(pFile))};
    GlyphData simpleGlyph{};
    if (const error_code errorCode{ReadGlyph(pFile, glyphLocations, compoundGlyph.glyphIndex, nrOfGlyphs, simpleGlyph)}; errorCode != no_error)
    {
        return errorCode;
    }
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    std::fseek(pFile, currentPosition, 0);

    for (std::uint32_t idx{}; idx < simpleGlyph.xPositions.size(); idx++)
    {
        simpleGlyph.xPositions[idx] = static_cast<std::uint16_t>((compoundGlyph.xx * simpleGlyph.xPositions[idx]) + (compoundGlyph.xy * simpleGlyph.yPositions[idx]) + compoundGlyph.argument1);
        simpleGlyph.yPositions[idx] = static_cast<std::uint16_t>((compoundGlyph.yx * simpleGlyph.xPositions[idx]) + (compoundGlyph.yy * simpleGlyph.yPositions[idx]) + compoundGlyph.argument2);
    }

    glyphData.xPositions.insert(glyphData.xPositions.end(), simpleGlyph.xPositions.cbegin(), simpleGlyph.xPositions.cend());
    glyphData.yPositions.insert(glyphData.yPositions.end(), simpleGlyph.yPositions.cbegin(), simpleGlyph.yPositions.cend());
    glyphData.flags.insert(glyphData.flags.end(), simpleGlyph.flags.cbegin(), simpleGlyph.flags.cend());
    glyphData.endPtsOfControus.insert(glyphData.endPtsOfControus.end(), simpleGlyph.endPtsOfControus.cbegin(), simpleGlyph.endPtsOfControus.cend());
    return no_error;
}

inline auto ReadCompoundGlyph(std::FILE* const pFile, const std::vector<std::uint32_t>& glyphLocations, const std::uint32_t numberOfGlyphs, GlyphData& glyphData) noexcept
{
    using enum error_code;
    bool MORE_COMPONENTS{};
    do
    {
        CompoundGlyph compoundGlyph{};
        if (ReadValue(pFile, compoundGlyph.flags) != no_error || ReadValue(pFile, compoundGlyph.glyphIndex) != no_error)
        {
            return font_parsing_error;
        }

        // ignore invalid glyph number
        if (compoundGlyph.glyphIndex >= numberOfGlyphs)
        {
            return invalid_font_format;
        }

        const bool ARG_1_AND_2_ARE_WORDS{(compoundGlyph.flags & 0x001) != 0};    // If set, the arguments are words;
                                                                                 // If not set, they are bytes.
        const bool ARGS_ARE_XY_VALUES{(compoundGlyph.flags & 0x002) != 0};       // If set, the arguments are xy values;
                                                                                 // If not set, they are points.
        const bool WE_HAVE_A_SCALE{(compoundGlyph.flags & 0x008) != 0};          // If set, there is a simple scale for the component.
                                                                                 // If not set, scale is 1.0.
        MORE_COMPONENTS = (compoundGlyph.flags & 0x020) != 0;                    // If set, at least one additional glyph follows this one.
        const bool WE_HAVE_AN_X_AND_Y_SCALE{(compoundGlyph.flags & 0x040) != 0}; // If set the x direction will use a different scale than the y direction.
        const bool WE_HAVE_A_TWO_BY_TWO{(compoundGlyph.flags & 0x080) != 0};     // If set there is a 2-by-2 transformation that will be used to scale the component.

        if (const error_code errorCode{ReadArguments(pFile, ARGS_ARE_XY_VALUES, ARG_1_AND_2_ARE_WORDS, compoundGlyph)}; errorCode != no_error)
        {
            return errorCode;
        }
        if (const error_code errorCode{ApplyScale(pFile, glyphLocations, numberOfGlyphs, WE_HAVE_A_SCALE, WE_HAVE_AN_X_AND_Y_SCALE, WE_HAVE_A_TWO_BY_TWO, compoundGlyph, glyphData)}; errorCode != no_error)
        {
            return errorCode;
        }

    } while (MORE_COMPONENTS);
    return no_error;
}

inline auto ReadGlyph(std::FILE* const pFile, const std::vector<std::uint32_t>& glyphLocations, const std::uint32_t glyphIndex, const std::uint32_t numberOfGlyphs, GlyphData& glyphData) noexcept -> error_code
{
    using enum error_code;
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, glyphLocations[glyphIndex], 0) != 0)
    {
        return font_parsing_error;
    }

    if (ReadValue(pFile, glyphData.description.numberOfContours) != no_error || ReadValue(pFile, glyphData.description.xMin) != no_error || ReadValue(pFile, glyphData.description.yMin) != no_error
        || ReadValue(pFile, glyphData.description.xMax) != no_error || ReadValue(pFile, glyphData.description.yMax) != no_error)
    {
        return font_parsing_error;
    }

    if (glyphData.description.numberOfContours > 0)
    {
        return ReadSimpleGlyph(pFile, glyphData.description.numberOfContours, glyphData);
    }
    if (glyphData.description.numberOfContours < 0)
    {
        return ReadCompoundGlyph(pFile, glyphLocations, numberOfGlyphs, glyphData);
    }

    return invalid_font_format;
}
inline auto ReadAllGlyphs(std::FILE* const pFile, const std::vector<std::uint32_t>& glyphLocations, const std::vector<std::pair<std::uint32_t, std::uint32_t> /*unused*/>& glyphMappings, std::vector<GlyphData>& glyphDatas)
{
    const std::uint32_t glyphLocationsSize{static_cast<std::uint32_t>(glyphLocations.size())};
    const std::size_t glyphMappingsSize{glyphMappings.size()};
    glyphDatas.resize(glyphMappingsSize);

    for (std::uint32_t idx{}; idx < glyphMappingsSize; ++idx)
    {
        const auto& [index, code]{glyphMappings[idx]};
        if (const error_code errorCode{ReadGlyph(pFile, glyphLocations, index, glyphLocationsSize, glyphDatas[idx])}; errorCode != error_code::no_error)
        {
            return errorCode;
        }
        glyphDatas[idx].glyphIndex   = index;
        glyphDatas[idx].unicodeValue = code;
    }
    return error_code::no_error;
}
