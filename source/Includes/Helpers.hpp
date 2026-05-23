#ifndef HOME_JORAN_DEV_BALBINO_SOURCE_ENGINE_RENDERER_FONT_SOURCE_INCLUDES_HELPERS_HPP
#define HOME_JORAN_DEV_BALBINO_SOURCE_ENGINE_RENDERER_FONT_SOURCE_INCLUDES_HELPERS_HPP

//
// Copyright (c) 2024.
// Author: Joran.
//

#pragma once
import FawnAlgebra;
import std;
using namespace fawn_algebra;

using shortFrac    = std::int16_t;
using Fixed        = std::int32_t;
using FWord        = std::int16_t;
using uFWord       = std::uint16_t;
using F2Dot14      = std::int16_t;
using longDateTime = std::int64_t;

enum class error_code : std::uint8_t
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
    requires((sizeof(T) <= sizeof(V)) && std::is_integral_v<T>)
auto ReadValue(std::FILE* const file, V& value)
{
    if (std::fread(&value, sizeof(T), 1, file) != 1)
    {
        return error_code::font_parsing_error;
    }
    if constexpr (std::endian::native == std::endian::little)
    {
        value = std::byteswap(value);
    }
    return error_code::no_error;
}
template <typename T>
    requires std::is_integral_v<T>
auto ReadValue(std::FILE* const file, T& value)
{
    return ReadValue<T, T>(file, value);
}
template <typename T>
    requires std::is_integral_v<T>
auto ReadValue(std::FILE* const file, std::vector<T>& values)
{
    if (std::fread(values.data(), sizeof(T), values.size(), file) != 1)
    {
        return error_code::font_parsing_error;
    }
    if constexpr (std::endian::native == std::endian::little)
    {
        for (auto& value : values)
        {
            value = std::byteswap(value);
        }
    }
    return error_code::no_error;
}

#endif
