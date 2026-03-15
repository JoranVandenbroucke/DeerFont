//
// Copyright (c) 2024.
// Author: Joran.
//

module;
#include <unordered_map>
#include <vector>
export module DeerFont.Font;
import FawnAlgebra.Arithmetics;
import FawnAlgebra.Bezier;
using namespace FawnAlgebra;

namespace DeerFont
{
export struct Glyph
{
    std::vector<bezierF2> curves;
    uint16                advanceWidth;
    int16                 leftSideBearing;
};
export struct Font
{
    std::unordered_map<uint, Glyph> glyphs;
};

export struct GlyphMesh
{
    std::vector<float2> vertices;
    std::vector<uint32> indices;
    uint32              fillSize;
};
export struct FontMesh
{
    std::unordered_map<uint, GlyphMesh> glyphs;
};
} // namespace DeerFont
