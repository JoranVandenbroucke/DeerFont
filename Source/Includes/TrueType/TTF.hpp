//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include "../Helpers.hpp"
import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

constexpr uint32 FourCharToUInt32(const char chars[4])
{
    return static_cast<uint32>(chars[0]) << 24 | static_cast<uint32>(chars[1]) << 16 | static_cast<uint32>(chars[2]) << 8 | static_cast<uint32>(chars[3]);
}

struct OffsetSubTable
{
    uint32 scalarType{};
    uint16 numTables{};
    uint16 searchRange{};
    uint16 entrySelector{};
    uint16 rangeShift{};
};
struct TableDirectory
{
    uint32 tag{};
    uint32 checksum{};
    uint32 offset{};
    uint32 length{};
};

constexpr uint32 g_cmapId{ FourCharToUInt32("cmap") };
constexpr uint32 g_glyfId{ FourCharToUInt32("glyf") };
constexpr uint32 g_headId{ FourCharToUInt32("head") };
constexpr uint32 g_hheaId{ FourCharToUInt32("hhea") };
constexpr uint32 g_hmtxId{ FourCharToUInt32("hmtx") };
constexpr uint32 g_locaId{ FourCharToUInt32("loca") };
constexpr uint32 g_maxpId{ FourCharToUInt32("maxp") };
constexpr uint32 g_nameId{ FourCharToUInt32("name") };
constexpr uint32 g_postId{ FourCharToUInt32("post") };

constexpr uint32 g_scalarType_True{ 0x74727565 };
constexpr uint32 g_scalarType_True_{ 0x00010000 };
constexpr uint32 g_scalarType_Typ1{ 0x74797031 };
constexpr uint32 g_scalarType_OTTO{ 0x4F54544F };

inline auto CalcTableChecksum(const uint32 *table, const uint32 numberOfBytesInTable) -> uint32
{
    uint32 sum    = 0;
    uint32 nLongs = (numberOfBytesInTable + 3) / 4;
    while ( nLongs-- > 0 )
        sum += *table++;
    return sum;
}

inline auto ReadOffsetSubTable(FILE *const file, OffsetSubTable &offsetSubTable) noexcept
{
    if ( ReadValue(file, offsetSubTable.scalarType) != error_code::no_error || ReadValue(file, offsetSubTable.numTables) != error_code::no_error || ReadValue(file, offsetSubTable.searchRange) != error_code::no_error ||
         ReadValue(file, offsetSubTable.entrySelector) != error_code::no_error || ReadValue(file, offsetSubTable.rangeShift) != error_code::no_error )
    {
        return error_code::font_parsing_error;
    }
    return error_code::no_error;
}

inline auto ReadTableDirectory(FILE *const file, const OffsetSubTable &offsetSubTable, std::vector<TableDirectory> &tableDirectory) noexcept
{
    tableDirectory.resize(offsetSubTable.numTables);
    for ( auto &[tag, checksum, offset, length] : tableDirectory )
    {
        if ( ReadValue(file, tag) != error_code::no_error || ReadValue(file, checksum) != error_code::no_error || ReadValue(file, offset) != error_code::no_error || ReadValue(file, length) != error_code::no_error )
        {
            return error_code::font_parsing_error;
        }
    }
    return error_code::no_error;
}
