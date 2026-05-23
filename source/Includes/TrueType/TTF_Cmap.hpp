//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

struct CMapIndex
{
    std::uint16_t version{};
    std::uint16_t numberSubTables{};
};
struct CMapSubTable
{
    std::uint16_t platformId{};
    std::uint16_t platformSpecificId{};
    std::uint32_t offset{};
};
struct Format4
{
    std::uint16_t length{};                                               // Length of subtable in bytes
    std::uint16_t language{};                                             // Language code (see above)
    std::uint16_t segCountX2{};                                           // 2 * segCount
    std::uint16_t searchRange{};                                          // 2 * (2**FLOOR(log2(segCount)))
    std::uint16_t entrySelector{};                                        // log2(searchRange/2)
    std::uint16_t rangeShift{};                                           // (2 * segCount) - searchRange
    std::vector<std::uint16_t> endCode{};                                 // Ending character code for each segment, last = 0xFFFF.
    std::uint16_t reservedPad{};                                          // This value should be zero
    std::vector<std::uint16_t> startCode{};                               // Starting character code for each segment
    std::vector<std::uint16_t> idDelta{};                                 // Delta for all character codes in segment
    std::vector<std::pair<std::uint32_t, std::uint16_t>> idRangeOffset{}; // Offset in bytes to glyph indexArray, or 0
    std::vector<std::uint16_t> glyphIndexArray{};                         // Glyph index array
};
struct Format12
{
    std::uint16_t reserved{};       // Set to 0.
    std::uint32_t length{};         // Byte length of this subtable (including the header)
    std::uint32_t language{};       // Language code (see above)
    std::uint32_t numberOfGroups{}; // Number of groupings which follow
};
constexpr std::uint16_t g_platformUnicode{0};
constexpr std::uint16_t g_platformMacintosh{1};
constexpr std::uint16_t g_platformMicrosoft{3};

constexpr std::uint16_t g_platformUnicode_version_1_0{0};
constexpr std::uint16_t g_platformUnicode_version_1_1{1};
constexpr std::uint16_t g_platformUnicode_iso{2}; // deprecated
constexpr std::uint16_t g_platformUnicode_unicode_2_0_BMP{3};
constexpr std::uint16_t g_platformUnicode_unicode_2_0{4}; // ignore
constexpr std::uint16_t g_platformUnicode_unicode{5};     // ignore
constexpr std::uint16_t g_platformUnicode_last_resort{6}; // ignore

/*
 * When the platformID is 1 (Macintosh), the platformSpecificID is a QuickDraw script code.
 * See the 'name' table documentation for a list of these.
 * The use of the Macintosh platformID is currently discouraged.
 * Subtables with a Macintosh platformID are only required for backwards compatibility with QuickDraw and will be synthesized from Unicode-based subtables if ever needed.
 */

constexpr std::uint16_t g_platformMicrosoft_Symbol{0};
constexpr std::uint16_t g_platformMicrosoft_Unicode_BMP{1};
constexpr std::uint16_t g_platformMicrosoft_Shift{2};
constexpr std::uint16_t g_platformMicrosoft_PRC{3};
constexpr std::uint16_t g_platformMicrosoft_BigFive{4};
constexpr std::uint16_t g_platformMicrosoft_Johab{5};
constexpr std::uint16_t g_platformMicrosoft_Unicode_UCS_4{10};

inline auto ReadFormat4Header(std::FILE* const pFile, Format4& format) noexcept -> error_code
{
    using enum error_code;
    if (ReadValue(pFile, format.length) != no_error || ReadValue(pFile, format.language) != no_error || ReadValue(pFile, format.segCountX2) != no_error)
    {
        return font_parsing_error;
    }
    const std::uint16_t segCount{static_cast<std::uint16_t>(format.segCountX2 / static_cast<std::uint16_t>(2))};
    // todo : 1 == SEEK_CUR, make sure it's no longer hard coded
    if (constexpr std::int32_t offset{6}; std::fseek(pFile, offset, 1) != 0) // Skip: searchRange, entrySelector, rangeShift
    {
        return font_parsing_error;
    }
    format.endCode.resize(segCount);
    format.startCode.resize(segCount);
    format.idDelta.resize(segCount);
    format.idRangeOffset.resize(segCount);
    return no_error;
}

inline auto ReadFormat4Segments(std::FILE* const pFile, Format4& format) noexcept -> error_code
{
    using enum error_code;

    if (ReadValue(pFile, format.endCode) != no_error)
    {
        return font_parsing_error;
    }
    // todo : 1 == SEEK_CUR, make sure it's no longer hard coded
    if (std::fseek(pFile, sizeof(format.reservedPad), 1) != 0)
    {
        return font_parsing_error;
    }
    if (ReadValue(pFile, format.startCode) != no_error || ReadValue(pFile, format.idDelta) != no_error)
    {
        return font_parsing_error;
    }

    for (auto& [readLock, offset] : format.idRangeOffset)
    {
        readLock = static_cast<std::uint32_t>(std::ftell(pFile));
        if (ReadValue(pFile, offset) != no_error)
        {
            return font_parsing_error;
        }
    }
    return no_error;
}

inline auto ReadGlyphIndex(std::FILE* const pFile, const std::uint32_t& glyphIndexArrayLocation, int& glyphIndex) noexcept -> error_code
{
    using enum error_code;
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, glyphIndexArrayLocation, 0) != 0 || ReadValue(pFile, glyphIndex) != no_error)
    {
        return font_parsing_error;
    }
    return no_error;
}

inline auto ProcessSingleGlyph(std::FILE* const pFile, const Format4& format, const std::size_t i, const std::uint16_t& currCode, int& glyphIndex) noexcept -> error_code
{
    if (format.idRangeOffset[i].second == 0)
    {
        glyphIndex = (currCode + format.idDelta[i]) % 65536;
        return error_code::no_error;
    }

    const std::uint32_t readerLocationOld{static_cast<std::uint32_t>(std::ftell(pFile))};
    const std::uint32_t rangeOffsetLocation{format.idRangeOffset[i].first + format.idRangeOffset[i].second};
    const std::uint32_t glyphIndexArrayLocation{2 * (currCode - format.startCode[i]) + rangeOffsetLocation};

    if (const error_code result{ReadGlyphIndex(pFile, glyphIndexArrayLocation, glyphIndex)}; result != error_code::no_error)
    {
        return result;
    }

    if (glyphIndex != 0)
    {
        constexpr std::uint32_t mask{std::numeric_limits<std::uint16_t>::max()};
        glyphIndex = (glyphIndex + format.idDelta[i]) & mask;
    }

    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, readerLocationOld, 0) != 0)
    {
        return error_code::font_parsing_error;
    }

    return error_code::no_error;
}

inline auto ProcessGlyphIndices(std::FILE* const pFile, const Format4& format, std::vector<std::pair<std::uint32_t, std::uint32_t> /*unused*/>& glyphMap) noexcept -> error_code
{
    bool hasReadMissingCharGlyph{};

    for (std::size_t i{}; i < format.startCode.size(); ++i)
    {
        std::uint16_t currCode{format.startCode[i]};
        const std::uint16_t endCode{format.endCode[i]};

        if (currCode == 65535)
        {
            break; // Hack to avoid out of bounds
        }

        while (currCode <= endCode)
        {
            int glyphIndex{};
            if (ProcessSingleGlyph(pFile, format, i, currCode, glyphIndex) != error_code::no_error)
            {
                return error_code::font_parsing_error;
            }

            glyphMap.emplace_back(glyphIndex, currCode);
            hasReadMissingCharGlyph |= glyphIndex == 0;
            ++currCode;
        }
    }

    return hasReadMissingCharGlyph ? error_code::invalid_font_format : error_code::no_error;
}

inline auto ParseFormat4(std::FILE* const pFile, std::vector<std::pair<std::uint32_t, std::uint32_t>>& glyphMap) noexcept
{
    using enum error_code;
    Format4 format{};
    if (ReadFormat4Header(pFile, format) != no_error)
    {
        return font_parsing_error;
    }

    if (ReadFormat4Segments(pFile, format) != no_error)
    {
        return font_parsing_error;
    }

    return ProcessGlyphIndices(pFile, format, glyphMap);
}

inline auto ParseFormat12(std::FILE* const pFile, std::vector<std::pair<std::uint32_t, std::uint32_t>>& glyphMap) noexcept
{
    using enum error_code;
    Format12 format{};
    if (ReadValue(pFile, format.reserved) != no_error || ReadValue(pFile, format.length) != no_error || ReadValue(pFile, format.language) != no_error || ReadValue(pFile, format.numberOfGroups) != no_error)
    {
        return font_parsing_error;
    }

    bool hasReadMissingCharGlyph{};
    for (std::uint32_t i{}; i < format.numberOfGroups; i++)
    {
        std::uint32_t startCharCode{};
        std::uint32_t endCharCode{};
        std::uint32_t startGlyphIndex{};
        if (ReadValue(pFile, startCharCode) != no_error || ReadValue(pFile, endCharCode) != no_error || ReadValue(pFile, startGlyphIndex) != no_error)
        {
            return font_parsing_error;
        }

        const std::uint32_t numChars{endCharCode - startCharCode + 1U};
        for (std::uint32_t charCodeOffset{}; charCodeOffset < numChars; ++charCodeOffset)
        {
            const std::uint32_t charCode{startCharCode + charCodeOffset};
            const std::uint32_t glyphIndex{startGlyphIndex + charCodeOffset};

            glyphMap.emplace_back(glyphIndex, charCode);
            hasReadMissingCharGlyph |= glyphIndex == 0;
        }
    }
    return hasReadMissingCharGlyph ? invalid_font_format : no_error;
}

inline auto ReadCmap(std::FILE* const pFile, const std::uint32_t cmapLocation, std::vector<std::pair<std::uint32_t, std::uint32_t> /*unused*/>& glyphMap) noexcept
{
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, cmapLocation, 0) != 0)
    {
        return error_code::font_parsing_error;
    }

    using enum error_code;
    // Read the cmap index.
    CMapIndex cmapIndex{};
    if (ReadValue(pFile, cmapIndex.version) != no_error || ReadValue(pFile, cmapIndex.numberSubTables) != no_error)
    {
        return font_parsing_error;
    }

    std::uint16_t selectedUnicodeVersionID{static_cast<std::uint16_t>(~0U)};
    std::uint32_t cmapSubtableOffset{};
    for (int i{}; i < cmapIndex.numberSubTables; ++i)
    {
        CMapSubTable subTable{};
        if (ReadValue(pFile, subTable.platformId) != no_error || ReadValue(pFile, subTable.platformSpecificId) != no_error || ReadValue(pFile, subTable.offset) != no_error)
        {
            return font_parsing_error;
        }

        if (subTable.platformId == g_platformUnicode)
        {
            if ((subTable.platformSpecificId == g_platformUnicode_version_1_0 || subTable.platformSpecificId == g_platformUnicode_version_1_1 || subTable.platformSpecificId == g_platformUnicode_unicode_2_0_BMP
                 || subTable.platformSpecificId == g_platformUnicode_unicode_2_0)
                && subTable.platformSpecificId > selectedUnicodeVersionID)
            {
                cmapSubtableOffset       = subTable.offset;
                selectedUnicodeVersionID = subTable.platformSpecificId;
            }
        }
        else if (subTable.platformId == g_platformMicrosoft && (subTable.platformSpecificId == g_platformMicrosoft_Unicode_BMP || subTable.platformSpecificId == g_platformMicrosoft_Unicode_UCS_4))
        {
            cmapSubtableOffset = subTable.offset;
        }
    }

    if (cmapSubtableOffset == 0)
    {
        return unsupported_font_type;
    }
    // todo : 0 == SEEK_SET, make sure it's no longer hard coded
    if (std::fseek(pFile, cmapLocation + cmapSubtableOffset, 0) != 0)
    {
        return font_parsing_error;
    }

    std::uint16_t format{};
    if (const error_code errorCode{ReadValue(pFile, format)}; errorCode != no_error)
    {
        return errorCode;
    }

    switch (format)
    {
    case 4: return ParseFormat4(pFile, glyphMap);
    case 12: return ParseFormat12(pFile, glyphMap);
    default: return unsupported_font_type;
    }
}
