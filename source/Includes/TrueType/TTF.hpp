#ifndef HOME_JORAN_DEV_BALBINO_SOURCE_ENGINE_RENDERER_FONT_SOURCE_INCLUDES_TRUETYPE_TTF_HPP
#define HOME_JORAN_DEV_BALBINO_SOURCE_ENGINE_RENDERER_FONT_SOURCE_INCLUDES_TRUETYPE_TTF_HPP

//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"

import FawnAlgebra;
import std;
using namespace fawn_algebra;

constexpr auto FourCharToUInt32(const char pChars[4]) -> std::uint32_t
{
    return static_cast<std::uint32_t>(pChars[0]) << 24 | static_cast<std::uint32_t>(pChars[1]) << 16 | static_cast<std::uint32_t>(pChars[2]) << 8 | static_cast<std::uint32_t>(pChars[3]);
}

struct OffsetSubTable
{
    std::uint32_t scalarType{};
    std::uint16_t numTables{};
    std::uint16_t searchRange{};
    std::uint16_t entrySelector{};
    std::uint16_t rangeShift{};
};
struct TableDirectory
{
    std::uint32_t tag{};
    std::uint32_t checksum{};
    std::uint32_t offset{};
    std::uint32_t length{};
};

constexpr std::uint32_t g_cmapId{FourCharToUInt32("cmap")};
constexpr std::uint32_t g_glyfId{FourCharToUInt32("glyf")};
constexpr std::uint32_t g_headId{FourCharToUInt32("head")};
constexpr std::uint32_t g_hheaId{FourCharToUInt32("hhea")};
constexpr std::uint32_t g_hmtxId{FourCharToUInt32("hmtx")};
constexpr std::uint32_t g_locaId{FourCharToUInt32("loca")};
constexpr std::uint32_t g_maxpId{FourCharToUInt32("maxp")};
constexpr std::uint32_t g_nameId{FourCharToUInt32("name")};
constexpr std::uint32_t g_postId{FourCharToUInt32("post")};

constexpr std::uint32_t g_scalarType_True1{0x74727565U};
constexpr std::uint32_t g_scalarType_True2{0x00010000U};
constexpr std::uint32_t g_scalarType_Typ1{0x74797031U};
constexpr std::uint32_t g_scalarType_OTTO{0x4F54544FU};

inline auto CalcTableChecksum(const std::uint32_t* pTable, const std::uint32_t numberOfBytesInTable) -> std::uint32_t
{
    std::uint32_t sum{0};
    std::uint32_t nLongs{(numberOfBytesInTable + 3) / 4};
    while (nLongs-- > 0)
    {
        sum += *pTable++;
    }
    return sum;
}

inline auto ReadOffsetSubTable(std::FILE* const pFile, OffsetSubTable& offsetSubTable) noexcept
{
    if (ReadValue(pFile, offsetSubTable.scalarType) != error_code::no_error || ReadValue(pFile, offsetSubTable.numTables) != error_code::no_error || ReadValue(pFile, offsetSubTable.searchRange) != error_code::no_error
        || ReadValue(pFile, offsetSubTable.entrySelector) != error_code::no_error || ReadValue(pFile, offsetSubTable.rangeShift) != error_code::no_error)
    {
        return error_code::font_parsing_error;
    }
    return error_code::no_error;
}

inline auto ReadTableDirectory(std::FILE* const pFile, const OffsetSubTable& offsetSubTable, std::vector<TableDirectory>& tableDirectory) noexcept
{
    tableDirectory.resize(offsetSubTable.numTables);
    for (auto& [tag, checksum, offset, length] : tableDirectory)
    {
        if (ReadValue(pFile, tag) != error_code::no_error || ReadValue(pFile, checksum) != error_code::no_error || ReadValue(pFile, offset) != error_code::no_error || ReadValue(pFile, length) != error_code::no_error)
        {
            return error_code::font_parsing_error;
        }
    }
    return error_code::no_error;
}

#endif
