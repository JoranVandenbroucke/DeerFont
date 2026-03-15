//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

#include <cstddef>

import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

struct CMapIndex
{
    uint16 version;
    uint16 numberSubTables;
};
struct CMapSubTable
{
    uint16 platformId;
    uint16 platformSpecificId;
    uint32 offset;
};
struct Format4
{
    uint16                                   length;          // Length of subtable in bytes
    uint16                                   language;        // Language code (see above)
    uint16                                   segCountX2;      // 2 * segCount
    uint16                                   searchRange;     // 2 * (2**FLOOR(log2(segCount)))
    uint16                                   entrySelector;   // log2(searchRange/2)
    uint16                                   rangeShift;      // (2 * segCount) - searchRange
    std::vector<uint16>                      endCode;         // Ending character code for each segment, last = 0xFFFF.
    uint16                                   reservedPad;     // This value should be zero
    std::vector<uint16>                      startCode;       // Starting character code for each segment
    std::vector<uint16>                      idDelta;         // Delta for all character codes in segment
    std::vector<std::pair<uint32, uint16>> idRangeOffset;   // Offset in bytes to glyph indexArray, or 0
    std::vector<uint16>                      glyphIndexArray; // Glyph index array
};
struct Format12
{
    uint16 reserved; // Set to 0.
    uint32 length;   // Byte length of this subtable (including the header)
    uint32 language; // Language code (see above)
    uint32 nGroups;  // Number of groupings which follow
};
constexpr uint16 g_platformUnicode{ 0 };
constexpr uint16 g_platformMacintosh{ 1 };
constexpr uint16 g_platformMicrosoft{ 3 };

constexpr uint16 g_platformUnicode_version_1_0{ 0 };
constexpr uint16 g_platformUnicode_version_1_1{ 1 };
constexpr uint16 g_platformUnicode_iso{ 2 }; // deprecated
constexpr uint16 g_platformUnicode_unicode_2_0_BMP{ 3 };
constexpr uint16 g_platformUnicode_unicode_2_0{ 4 }; // ignore
constexpr uint16 g_platformUnicode_unicode{ 5 };     // ignore
constexpr uint16 g_platformUnicode_last_resort{ 6 }; // ignore

/*
 * When the platformID is 1 (Macintosh), the platformSpecificID is a QuickDraw script code.
 * See the 'name' table documentation for a list of these.
 * The use of the Macintosh platformID is currently discouraged.
 * Subtables with a Macintosh platformID are only required for backwards compatibility with QuickDraw and will be synthesized from Unicode-based subtables if ever needed.
 */


constexpr uint16 g_platformMicrosoft_Symbol{ 0 };
constexpr uint16 g_platformMicrosoft_Unicode_BMP{ 1 };
constexpr uint16 g_platformMicrosoft_Shift{ 2 };
constexpr uint16 g_platformMicrosoft_PRC{ 3 };
constexpr uint16 g_platformMicrosoft_BigFive{ 4 };
constexpr uint16 g_platformMicrosoft_Johab{ 5 };
constexpr uint16 g_platformMicrosoft_Unicode_UCS_4{ 10 };

inline error_code ReadFormat4Header(FILE *const file, Format4 &format) noexcept
{
    using enum error_code;
    if ( ReadValue(file, format.length) != no_error || ReadValue(file, format.language) != no_error || ReadValue(file, format.segCountX2) != no_error )
    {
        return font_parsing_error;
    }
    const uint16 segCount = format.segCountX2 / 2;
    if ( fseek(file, 6, SEEK_CUR) != 0 ) // Skip: searchRange, entrySelector, rangeShift
    {
        return font_parsing_error;
    }
    format.endCode.resize(segCount);
    format.startCode.resize(segCount);
    format.idDelta.resize(segCount);
    format.idRangeOffset.resize(segCount);
    return no_error;
}

inline error_code ReadFormat4Segments(FILE *const file, Format4 &format) noexcept
{
    using enum error_code;

    if ( ReadValue(file, format.endCode) != no_error)
    {
        return font_parsing_error;
    }
    if ( fseek(file, sizeof(format.reservedPad), SEEK_CUR) != 0 )
    {
        return font_parsing_error;
    }
    if ( ReadValue(file, format.startCode) != no_error || ReadValue(file, format.idDelta) != no_error )
    {
        return font_parsing_error;
    }

    for ( auto &[readLock, offset] : format.idRangeOffset )
    {
        readLock = ftell(file);
        if ( ReadValue(file, offset) != no_error )
        {
            return font_parsing_error;
        }
    }
    return no_error;
}

inline error_code ReadGlyphIndex(FILE *const file, const uint32 &glyphIndexArrayLocation, int &glyphIndex) noexcept
{
    using enum error_code;
    if ( fseek(file, glyphIndexArrayLocation, SEEK_SET) != 0 || ReadValue(file, glyphIndex) != no_error )
    {
        return font_parsing_error;
    }
    return no_error;
}

inline error_code ProcessSingleGlyph(FILE *const file, const Format4 &format, const std::size_t i, const uint16 &currCode, int &glyphIndex) noexcept
{
    if ( format.idRangeOffset[i].second == 0 )
    {
        glyphIndex = (currCode + format.idDelta[i]) % 65536;
        return error_code::no_error;
    }

    const uint32 readerLocationOld{ static_cast<uint32>(ftell(file)) };
    const uint32 rangeOffsetLocation{ format.idRangeOffset[i].first + format.idRangeOffset[i].second };
    const uint32 glyphIndexArrayLocation{ 2 * (currCode - format.startCode[i]) + rangeOffsetLocation };

    if ( const error_code result = ReadGlyphIndex(file, glyphIndexArrayLocation, glyphIndex); result != error_code::no_error )
        return result;

    if ( glyphIndex != 0 )
    {
        glyphIndex = (glyphIndex + format.idDelta[i]) % 65536;
    }

    if ( fseek(file, readerLocationOld, SEEK_SET) != 0 )
    {
        return error_code::font_parsing_error;
    }

    return error_code::no_error;
}

inline error_code ProcessGlyphIndices(FILE *const file, const Format4 &format, std::vector<std::pair<uint32, uint32>> &glyphMap) noexcept
{
    bool hasReadMissingCharGlyph = false;

    for ( std::size_t i = 0; i < format.startCode.size(); ++i )
    {
        uint16       currCode = format.startCode[i];
        const uint16 endCode  = format.endCode[i];

        if ( currCode == 65535 )
            break; // Hack to avoid out of bounds

        while ( currCode <= endCode )
        {
            int glyphIndex = 0;
            if ( ProcessSingleGlyph(file, format, i, currCode, glyphIndex) != error_code::no_error )
            {
                return error_code::font_parsing_error;
            }

            glyphMap.emplace_back(glyphIndex, currCode);
            hasReadMissingCharGlyph |= (glyphIndex == 0);
            ++currCode;
        }
    }

    return hasReadMissingCharGlyph ? error_code::invalid_font_format : error_code::no_error;
}


inline auto ParseFormat4(FILE *const file, std::vector<std::pair<uint32, uint32>> &glyphMap) noexcept
{
    using enum error_code;
    Format4 format;
    if ( ReadFormat4Header(file, format) != no_error )
    {
        return font_parsing_error;
    }

    if ( ReadFormat4Segments(file, format) != no_error )
    {
        return font_parsing_error;
    }

    return ProcessGlyphIndices(file, format, glyphMap);
}

inline auto ParseFormat12(FILE *const file, std::vector<std::pair<uint32, uint32>> &glyphMap) noexcept
{
    using enum error_code;
    Format12 format;
    if ( ReadValue(file, format.reserved) != no_error || ReadValue(file, format.length) != no_error || ReadValue(file, format.language) != no_error || ReadValue(file, format.nGroups) != no_error )
    {
        return font_parsing_error;
    }

    bool hasReadMissingCharGlyph{};
    for ( int i = 0; i < format.nGroups; i++ )
    {
        uint32 startCharCode;
        uint32 endCharCode;
        uint32 startGlyphIndex;
        if ( ReadValue(file, startCharCode) != no_error || ReadValue(file, endCharCode) != no_error || ReadValue(file, startGlyphIndex) != no_error )
        {
            return font_parsing_error;
        }

        const uint32 numChars{ endCharCode - startCharCode + 1U };
        for ( uint32 charCodeOffset{}; charCodeOffset < numChars; ++charCodeOffset )
        {
            const uint32 charCode{ startCharCode + charCodeOffset };
            const uint32 glyphIndex{ startGlyphIndex + charCodeOffset };

            glyphMap.emplace_back(glyphIndex, charCode);
            hasReadMissingCharGlyph |= glyphIndex == 0;
        }
    }
    return hasReadMissingCharGlyph ? invalid_font_format : no_error;
}

inline auto ReadCmap(FILE *const file, const uint32 cmapLocation, std::vector<std::pair<uint32, uint32>> &glyphMap) noexcept
{
    if ( fseek(file, cmapLocation, SEEK_SET) != 0 )
    {
        return error_code::font_parsing_error;
    }

    using enum error_code;
    // Read the cmap index.
    CMapIndex cmapIndex;
    if ( ReadValue(file, cmapIndex.version) != no_error || ReadValue(file, cmapIndex.numberSubTables) != no_error )
    {
        return font_parsing_error;
    }

    int16  selectedUnicodeVersionID{ -1 };
    uint32 cmapSubtableOffset{};
    for ( int i = 0; i < cmapIndex.numberSubTables; ++i )
    {
        CMapSubTable subTable;
        if ( ReadValue(file, subTable.platformId) != no_error || ReadValue(file, subTable.platformSpecificId) != no_error || ReadValue(file, subTable.offset) != no_error )
        {
            return font_parsing_error;
        }

        if ( subTable.platformId == g_platformUnicode )
        {
            if ( subTable.platformSpecificId == g_platformUnicode_version_1_0 || subTable.platformSpecificId == g_platformUnicode_version_1_1 || subTable.platformSpecificId == g_platformUnicode_unicode_2_0_BMP ||
                 subTable.platformSpecificId == g_platformUnicode_unicode_2_0 && subTable.platformSpecificId > selectedUnicodeVersionID )
            {
                cmapSubtableOffset       = subTable.offset;
                selectedUnicodeVersionID = subTable.platformSpecificId;
            }
        }
        else if ( subTable.platformId == g_platformMicrosoft && (subTable.platformSpecificId == g_platformMicrosoft_Unicode_BMP || subTable.platformSpecificId == g_platformMicrosoft_Unicode_UCS_4) )
        {
            cmapSubtableOffset = subTable.offset;
        }
    }

    if ( cmapSubtableOffset == 0 )
    {
        return unsupported_font_type;
    }
    if ( fseek(file, cmapLocation + cmapSubtableOffset, SEEK_SET) != 0 )
    {
        return font_parsing_error;
    }
    uint16 format;
    if ( const error_code errorCode = ReadValue(file, format); errorCode != no_error )
    {
        return errorCode;
    }

    switch ( format )
    {
        case 4:
            return ParseFormat4(file, glyphMap);
        case 12:
            return ParseFormat12(file, glyphMap);
        default:
            return unsupported_font_type;
    }
}
