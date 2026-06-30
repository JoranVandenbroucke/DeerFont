//
// Copyright (c) 2024.
// Author: Joran.
//

module;
#include "TrueType/TTF.hpp"
#include "TrueType/TTF_Cmap.hpp"
#include "TrueType/TTF_Glyf.hpp"
#include "TrueType/TTF_Head.hpp"
#include "TrueType/TTF_Hhea.hpp"
#include "TrueType/TTF_Hmtx.hpp"
#include "TrueType/TTF_Loca.hpp"
#include "TrueType/TTF_Maxp.hpp"
export module DeerFont:TrueType;
import :Font;

import FawnAlgebra;
import std;
using namespace fawn_algebra;

namespace DeerFont
{
void g_ApplyLayoutInfo(std::vector<GlyphData>& glyphDatas, const Hmtx& hmtx)
{
    for (auto& [endPtsOfControus, xPositions, yPositions, flags, description, unicodeValue, glyphIndex, advanceWidth, leftSideBearing] : glyphDatas)
    {
        advanceWidth    = hmtx.hMetrics[glyphIndex].advanceWidth;
        leftSideBearing = hmtx.hMetrics[glyphIndex].leftSideBearing;
    }
}
void ConvertDataToFont(const std::vector<GlyphData>& glyphDatas, Font& font)
{
    for (const auto& [endPtsOfControus, xPositions, yPositions, flags, description, unicodeValue, glyphIndex, advanceWidth, leftSideBearing] : glyphDatas)
    {
        std::uint16_t lastIdx{};
        Glyph glyph;
        glyph.curves.emplace_back();
        const std::size_t xPointsSize{xPositions.size()};
        for (std::uint32_t idx{0}; idx < xPointsSize; ++idx)
        {
            if (idx > endPtsOfControus[lastIdx])
            {
                ++lastIdx;
                Close(glyph.curves.back());
                glyph.curves.emplace_back();
            }
            const float2 pos{static_cast<float>(xPositions[idx]) / static_cast<float>(1 << 14), static_cast<float>(yPositions[idx]) / static_cast<float>(1 << 14)};
            if (IsOnCurve(flags[idx]))
            {
                AddPoint(glyph.curves.back(), pos);
            }
            else
            {
                AddControlPoint(glyph.curves.back(), pos);
            }
        }
        glyph.advanceWidth    = advanceWidth;
        glyph.leftSideBearing = leftSideBearing;
        font.glyphs.insert(std::make_pair(unicodeValue, glyph));
    }
}

constexpr auto FopenS(std::FILE** ppFile, const char* pName, const char* pMode) -> int
{
    *ppFile = std::fopen(pName, pMode);
    return *ppFile ? 0 : -1;
}

export auto ReadFont(const char8_t* const pFilePath, Font& font) noexcept
{
    std::FILE* pFile;
    FopenS(&pFile, std::bit_cast<const char* const>(pFilePath), "rb");
    if (!pFile)
    {
        // can't open pFile
        return -1;
    }

    OffsetSubTable subTable;
    if (ReadOffsetSubTable(pFile, subTable) != error_code::no_error)
    {
        // reading problem/incorrect format
        return -1;
    }

    std::vector<TableDirectory> tableDirectories{};
    if (ReadTableDirectory(pFile, subTable, tableDirectories) != error_code::no_error)
    {
        // reading problem/incorrect format
        return -1;
    }

    std::uint32_t cmapLocation{};
    std::uint32_t glypLocation{};
    std::uint32_t headLocation{};
    std::uint32_t hheaLocation{};
    std::uint32_t hmtxLocation{};
    std::uint32_t locaLocation{};
    std::uint32_t maxpLocation{};
    for (const auto& [tag, checksum, offset, length] : tableDirectories)
    {
        switch (tag)
        {
        case g_cmapId: cmapLocation = offset; break;
        case g_glyfId: glypLocation = offset; break;
        case g_headId: headLocation = offset; break;
        case g_hheaId: hheaLocation = offset; break;
        case g_hmtxId: hmtxLocation = offset; break;
        case g_locaId: locaLocation = offset; break;
        case g_maxpId: maxpLocation = offset; break;
        default: continue;
        }
        if (cmapLocation != 0 && glypLocation != 0 && headLocation != 0 && hheaLocation != 0 && hmtxLocation != 0 && locaLocation != 0 && maxpLocation != 0)
        {
            break;
        }
    }

    // not supported font
    if (cmapLocation == 0 || glypLocation == 0 || headLocation == 0 || hheaLocation == 0 || hmtxLocation == 0 || locaLocation == 0 || maxpLocation == 0)
    {
        return -1;
    }

    Head head;
    if (ReadHeader(pFile, headLocation, head) != error_code::no_error)
    {
        return -1;
    }

    Maxp maxp;
    if (ReadMaxp(pFile, maxpLocation, maxp) != error_code::no_error)
    {
        return -1;
    }

    std::vector<std::uint32_t> glyphLocations;
    if (ReadLoca(pFile, locaLocation, maxp.numGlyphs, glypLocation, head.indexToLocFormat == 0, glyphLocations) != error_code::no_error)
    {
        return -1;
    }

    std::vector<std::pair<std::uint32_t, std::uint32_t>> glyphMapping;
    if (ReadCmap(pFile, cmapLocation, glyphMapping) != error_code::no_error)
    {
        return -1;
    }

    std::vector<GlyphData> glyphDatas;
    if (ReadAllGlyphs(pFile, glyphLocations, glyphMapping, glyphDatas) != error_code::no_error)
    {
        return -1;
    }

    std::uint16_t numOfLongHorMetrics;
    if (GetNumAdvanceWidthMetrics(pFile, hheaLocation, numOfLongHorMetrics) != error_code::no_error)
    {
        return -1;
    }

    Hmtx hmtx;
    if (GetHorizontalLayoutInformation(pFile, hmtxLocation, numOfLongHorMetrics, static_cast<std::uint16_t>(glyphLocations.size()), hmtx) != error_code::no_error)
    {
        return -1;
    }
    g_ApplyLayoutInfo(glyphDatas, hmtx);
    ConvertDataToFont(glyphDatas, font);
    return 0;
}

// todo: use a different meshing algorithm and profile them eg. Hertel-Mehlhorn, Delaunay triangulation, or
// todo: check if casing is better than leading when sting changes, find ballence between cpu speeds and ram usage
export template <typename T, std::size_t N>
auto FontToMesh(const Font& font, const std::string_view unicodeStr, FontMesh& fontMesh)
{
    std::vector<std::uint32_t> usedChars;
    std::size_t idx = 0;
    while (idx < unicodeStr.size())
    {
        std::uint32_t codePoint{0};

        if (const std::uint8_t c = static_cast<std::uint8_t>(unicodeStr[idx]); c <= std::uint8_t{0x7F})
        { // 1-byte ASCII
            codePoint  = static_cast<std::uint32_t>(c);
            idx       += std::size_t{1};
        }
        else if ((c & std::uint8_t{0xE0}) == std::uint8_t{0xC0})
        { // 2-byte code point
            const std::uint32_t b1{static_cast<std::uint8_t>(unicodeStr[idx + std::size_t{1}])};
            codePoint  = (static_cast<std::uint32_t>(c & std::uint8_t{0x1F}) << 6U) | (b1 & 0x3FU);
            idx       += std::size_t{2};
        }
        else if ((c & std::uint8_t{0xF0}) == std::uint8_t{0xE0})
        { // 3-byte code point
            const std::uint32_t b1{static_cast<std::uint8_t>(unicodeStr[idx + std::size_t{1}])};
            const std::uint32_t b2{static_cast<std::uint8_t>(unicodeStr[idx + std::size_t{2}])};
            codePoint  = (static_cast<std::uint32_t>(c & std::uint8_t{0x0F}) << 12U) | ((b1 & 0x3FU) << 6U) | (b2 & 0x3FU);
            idx       += std::size_t{3};
        }
        else if ((c & std::uint8_t{0xF8}) == std::uint8_t{0xF0})
        { // 4-byte code point
            const std::uint32_t b1{static_cast<std::uint8_t>(unicodeStr[idx + std::size_t{1}])};
            const std::uint32_t b2{static_cast<std::uint8_t>(unicodeStr[idx + std::size_t{2}])};
            const std::uint32_t b3{static_cast<std::uint8_t>(unicodeStr[idx + std::size_t{3}])};
            codePoint  = (static_cast<std::uint32_t>(c & std::uint8_t{0x07}) << 18U) | ((b1 & 0x3FU) << 12U) | ((b2 & 0x3FU) << 6U) | (b3 & 0x3FU);
            idx       += std::size_t{4};
        }
        else
        {
            break;
        }
        if (std::ranges::find(usedChars, codePoint) != usedChars.cend() || !font.glyphs.contains(codePoint))
        {
            continue;
        }
        usedChars.emplace_back(codePoint);

        // Get the glyph
        std::vector<Vec<T, N>> glyphOddVertices;
        std::vector<Vec<T, N>> glyphArcVertices;
        std::vector<int> glyphOddIndices;
        std::vector<int> glyphArcIndices;
        for (const Glyph& glyph{font.glyphs.at(codePoint)}; const auto& bezier : glyph.curves)
        {
            for (std::size_t bezierPointId{}; bezierPointId < bezier.size(); bezierPointId += 2)
            {
                glyphOddVertices.push_back(bezier[bezierPointId]);
                glyphArcVertices.push_back(bezier[bezierPointId]);
                glyphArcVertices.push_back(bezier[(bezierPointId + 1) % bezier.size()]);
            }
        }
        EarClipping(glyphOddVertices, glyphOddIndices);

        for (std::size_t bezierPointId{}; bezierPointId < glyphArcVertices.size(); bezierPointId += 2)
        {
            glyphArcIndices.push_back(static_cast<int>(bezierPointId));
            glyphArcIndices.push_back(static_cast<int>((bezierPointId + 1) % glyphArcVertices.size()));
            glyphArcIndices.push_back(static_cast<int>((bezierPointId + 2) % glyphArcVertices.size()));
        }
        auto& [vertices, indices, fillSize]{fontMesh.glyphs[codePoint]};
        vertices.insert(vertices.end(), glyphOddVertices.cbegin(), glyphOddVertices.cend());
        vertices.insert(vertices.end(), glyphArcVertices.cbegin(), glyphArcVertices.cend());
        indices.insert(indices.end(), glyphOddIndices.cbegin(), glyphOddIndices.cend());
        indices.insert(indices.end(), glyphArcIndices.cbegin(), glyphArcIndices.cend());
        fillSize = static_cast<std::uint32_t>(glyphOddIndices.size());
    }
}
} // namespace DeerFont
