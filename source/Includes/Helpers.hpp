//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
#include <bit>
#include <cstdio>
#include <ostream>
#include <vector>

import FawnAlgebra.Arithmetics;
using namespace FawnAlgebra;

using shortFrac    = int16;
using Fixed        = int32;
using FWord        = int16;
using uFWord       = uint16;
using F2Dot14      = int16;
using longDateTime = int64;

enum class error_code : uint8
{
    no_error = 0,
    font_not_found,
    invalid_font_format,
    corrupted_font_file,
    unsupported_font_type,
    permission_denied,
    memory_allocation_failed,
    font_parsing_error
};

template <typename T, typename V>
    requires(sizeof(T) <= sizeof(V)) and std::is_integral_v<T>
auto ReadValue(FILE *const file, V &value)
{
    if ( std::fread(&value, sizeof(T), 1, file) != 1 )
    {
        return error_code::font_parsing_error;
    }
    if constexpr ( std::endian::native == std::endian::little )
    {
        value = std::byteswap(value);
    }
    return error_code::no_error;
}
template <typename T>
    requires std::is_integral_v<T>
auto ReadValue(FILE *const file, T &value)
{
    return ReadValue<T, T>(file, value);
}
template <typename T>
    requires std::is_integral_v<T>
auto ReadValue(FILE *const file, std::vector<T> &values)
{
    if ( std::fread(values.data(), sizeof(T), values.size(), file) != 1 )
    {
        return error_code::font_parsing_error;
    }
    if constexpr ( std::endian::native == std::endian::little )
    {
        for ( auto &value : values )
        {
            value = std::byteswap(value);
        }
    }
    return error_code::no_error;
}
