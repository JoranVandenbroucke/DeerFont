//
// Copyright (c) 2024.
// Author: Joran.
//

module;
export module DeerFont:Font;

import FawnAlgebra;
import std;

namespace DeerFont
{
export struct Glyph
{
    std::vector<fawn_algebra::dynamic_bezierF2> curves{};
    std::uint16_t advanceWidth{};
    std::int16_t leftSideBearing{};
};
export struct Font
{
    std::unordered_map<std::uint32_t, Glyph> glyphs{};
};

export struct GlyphMesh
{
    std::vector<fawn_algebra::float2> vertices{};
    std::vector<std::uint32_t> indices{};
    std::uint32_t fillSize{};
};
export struct FontMesh
{
    std::unordered_map<std::uint32_t, GlyphMesh> glyphs{};
};
} // namespace DeerFont
