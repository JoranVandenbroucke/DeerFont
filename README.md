# DeerFont

![Build Status](https://github.com/JoranVandenbroucke/DeerFont/actions/workflows/main.yml/badge.svg?branch=main)

DeerFont is a font parsing library designed for integration into the **Balbino** game engine.
It supports parsing various font formats, with current support for TrueType Fonts (TTF).
The library encodes glyphs using Bézier curves for uniformity and aims to expand support to OpenType (OTF) and Bitmap Font (FNT) formats.

## Features
- **TTF Parsing:** Support for parsing glyphs and extracting font metrics like width advance and left side bearing.
- **Bézier Curves:** Glyphs are encoded as Bézier curves to ensure a consistent representation.
- **Integration with Balbino:** Built to be implemented within the Balbino game engine.
- **Upcoming Support:** OTF and FNT formats will be added next.


## Installation

To build the project, ensure you have CMake installed and run the following commands:

```bash
cmake -B build -S .
cmake --build build
```