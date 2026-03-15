//
// Copyright (c) 2024.
// Author: Joran.
//

module;
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdio>
#include <unordered_map>
#include <vector>

#include "TrueType/TTF.hpp"
#include "TrueType/TTF_Cmap.hpp"
#include "TrueType/TTF_Glyf.hpp"
#include "TrueType/TTF_Head.hpp"
#include "TrueType/TTF_Hhea.hpp"
#include "TrueType/TTF_Hmtx.hpp"
#include "TrueType/TTF_Loca.hpp"
#include "TrueType/TTF_Maxp.hpp"
export module DeerFont.TrueType;
import FawnAlgebra.Arithmetics;
import FawnAlgebra.Bezier;
import DeerFont.Font;
using namespace FawnAlgebra;

namespace DeerFont
{
void ApplyLayoutInfo(std::vector<GlyphData> &glyphDatas, const Hmtx &hmtx)
{
    for ( auto &[endPtsOfControus, xPositions, yPositions, flags, description, unicodeValue, glyphIndex, advanceWidth, leftSideBearing] : glyphDatas )
    {
        advanceWidth    = hmtx.hMetrics[glyphIndex].advanceWidth;
        leftSideBearing = hmtx.hMetrics[glyphIndex].leftSideBearing;
    }
}
void ConvertDataToFont(const std::vector<GlyphData> &glyphDatas, Font &font)
{
    for ( const auto &[endPtsOfControus, xPositions, yPositions, flags, description, unicodeValue, glyphIndex, advanceWidth, leftSideBearing] : glyphDatas )
    {
        int16 lastIdx{};
        Glyph glyph;
        glyph.curves.emplace_back();
        const std::size_t xPointsSize{xPositions.size()};
        for ( int i{ 0 }; i < xPointsSize; ++i )
        {
            if ( i > endPtsOfControus[lastIdx] )
            {
                ++lastIdx;
                close(glyph.curves.back());
                glyph.curves.emplace_back();
            }
            const float2 pos{ static_cast<float>(xPositions[i]) / static_cast<float>(1 << 14), static_cast<float>(yPositions[i]) / static_cast<float>(1 << 14) };
            if ( isOnCurve(flags[i]) )
            {
                addPoint(glyph.curves.back(), pos);
            }
            else
            {
                addControlPoint(glyph.curves.back(), pos);
            }
        }
        glyph.advanceWidth    = advanceWidth;
        glyph.leftSideBearing = leftSideBearing;
        font.glyphs.insert(std::make_pair(unicodeValue, glyph));
    }
}

export auto ReadFont(const char8_t *const filePath, Font &font) noexcept
{
    FILE *file;
    fopen_s(&file, std::bit_cast<const char *const>(filePath), "rb");
    if ( !file )
    {
        // can't open file
        return -1;
    }

    OffsetSubTable subTable;
    if ( ReadOffsetSubTable(file, subTable) != error_code::no_error )
    {
        // reading problem/incorrect format
        return -1;
    }

    std::vector<TableDirectory> tableDirectories{};
    if ( ReadTableDirectory(file, subTable, tableDirectories) != error_code::no_error )
    {
        // reading problem/incorrect format
        return -1;
    }

    uint32 cmapLocation{};
    uint32 glypLocation{};
    uint32 headLocation{};
    uint32 hheaLocation{};
    uint32 hmtxLocation{};
    uint32 locaLocation{};
    uint32 maxpLocation{};
    for ( const auto &[tag, checksum, offset, length] : tableDirectories )
    {
        switch ( tag )
        {
            case g_cmapId:
                cmapLocation = offset;
                break;
            case g_glyfId:
                glypLocation = offset;
                break;
            case g_headId:
                headLocation = offset;
                break;
            case g_hheaId:
                hheaLocation = offset;
                break;
            case g_hmtxId:
                hmtxLocation = offset;
                break;
            case g_locaId:
                locaLocation = offset;
                break;
            case g_maxpId:
                maxpLocation = offset;
                break;
            default:
                continue;
        }
        if ( cmapLocation != 0 && glypLocation != 0 && headLocation != 0 && hheaLocation != 0 && hmtxLocation != 0 && locaLocation != 0 && maxpLocation != 0 )
        {
            break;
        }
    }

    // not supported font
    if ( cmapLocation == 0 || glypLocation == 0 || headLocation == 0 || hheaLocation == 0 || hmtxLocation == 0 || locaLocation == 0 || maxpLocation == 0 )
    {
        return -1;
    }

    Head head;
    if ( ReadHeader(file, headLocation, head) != error_code::no_error )
    {
        return -1;
    }

    Maxp maxp;
    if ( ReadMaxp(file, maxpLocation, maxp) != error_code::no_error )
    {
        return -1;
    }

    std::vector<uint32> glyphLocations;
    if ( ReadLoca(file, locaLocation, maxp.numGlyphs, glypLocation, head.indexToLocFormat == 0, glyphLocations) != error_code::no_error )
    {
        return -1;
    }

    std::vector<std::pair<uint32, uint32>> glyphMapping;
    if ( ReadCmap(file, cmapLocation, glyphMapping) != error_code::no_error )
    {
        return -1;
    }

    std::vector<GlyphData> glyphDatas;
    if ( ReadAllGlyphs(file, glyphLocations, glyphMapping, glyphDatas) != error_code::no_error )
    {
        return -1;
    }

    uint16 numOfLongHorMetrics;
    if ( GetNumAdvanceWidthMetrics(file, hheaLocation, numOfLongHorMetrics) != error_code::no_error )
    {
        return -1;
    }

    Hmtx hmtx;
    if ( GetHorizontalLayoutInformation(file, hmtxLocation, numOfLongHorMetrics, static_cast<uint32>(glyphLocations.size()), hmtx) != error_code::no_error )
    {
        return -1;
    }
    ApplyLayoutInfo(glyphDatas, hmtx);
    ConvertDataToFont(glyphDatas, font);
    return 0;
}

// todo: use a different meshing algorithm and profile them eg. Hertel-Mehlhorn, Delaunay triangulation, or
// todo: check if casing is better than leading when sting changes, find ballence between cpu speeds and ram usage
export template <typename T, std::size_t N>
auto FontToMesh(const Font &font, const std::string_view unicodeStr, FontMesh &fontMesh)
{
    std::vector<uint32> usedChars;
    std::size_t         i = 0;
    while ( i < unicodeStr.size() )
    {
        uint32_t codePoint{ 0 };

        if ( const unsigned char c{ static_cast<unsigned char>(unicodeStr[i]) }; c <= 0x7F )
        { // 1-byte ASCII
            codePoint = c;
            i += 1;
        }
        else if ( (c & 0xE0) == 0xC0 )
        { // 2-byte code point
            codePoint = ((c & 0x1F) << 6) | (unicodeStr[i + 1] & 0x3F);
            i += 2;
        }
        else if ( (c & 0xF0) == 0xE0 )
        { // 3-byte code point
            codePoint = ((c & 0x0F) << 12) | ((unicodeStr[i + 1] & 0x3F) << 6) | (unicodeStr[i + 2] & 0x3F);
            i += 3;
        }
        else if ( (c & 0xF8) == 0xF0 )
        { // 4-byte code point
            codePoint = ((c & 0x07) << 18) | ((unicodeStr[i + 1] & 0x3F) << 12) | ((unicodeStr[i + 2] & 0x3F) << 6) | (unicodeStr[i + 3] & 0x3F);
            i += 4;
        }
        else
        {
            break;
        }
        if ( std::ranges::find(usedChars, codePoint) != usedChars.cend() || !font.glyphs.contains(codePoint) )
        {
            continue;
        }
        usedChars.emplace_back(codePoint);

        // Get the glyph
        std::vector<Vec<T, N>> glyphOddVertices;
        std::vector<Vec<T, N>> glyphArcVertices;
        std::vector<int>       glyphOddIndices;
        std::vector<int>       glyphArcIndices;
        for ( const Glyph &glyph{ font.glyphs.at(codePoint) }; const auto &bezier : glyph.curves )
        {
            for ( std::size_t j{}; j < bezier.points.size(); j += 2 )
            {
                glyphOddVertices.push_back(bezier.points[j]);
                glyphArcVertices.push_back(bezier.points[j]);
                glyphArcVertices.push_back(bezier.points[(j + 1) % bezier.points.size()]);
            }
        }
        EarClipping(glyphOddVertices, glyphOddIndices);

        for ( std::size_t j{}; j < glyphArcVertices.size(); j += 2 )
        {
            glyphArcIndices.push_back(j);
            glyphArcIndices.push_back((j + 1) % glyphArcVertices.size());
            glyphArcIndices.push_back((j + 2) % glyphArcVertices.size());
        }
        auto &[vertices, indices, fillSize]{ fontMesh.glyphs[codePoint] };
        vertices.insert(vertices.end(), glyphOddVertices.cbegin(), glyphOddVertices.cend());
        vertices.insert(vertices.end(), glyphArcVertices.cbegin(), glyphArcVertices.cend());
        indices.insert(indices.end(), glyphOddIndices.cbegin(), glyphOddIndices.cend());
        indices.insert(indices.end(), glyphArcIndices.cbegin(), glyphArcIndices.cend());
        fillSize = glyphOddIndices.size();
    }
}
} // namespace DeerFont
