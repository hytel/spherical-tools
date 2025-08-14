/*-----------------------------------------------------------------------------
The MIT License

Copyright © 2025-present Hillel Steinberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------*/

#ifndef IMAGE_TO_CUBEMAP_HPP
#define IMAGE_TO_CUBEMAP_HPP

// Use a more standard PI definition for better portability
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/***********************************************************************/

// Use a pragma to ensure the structures are tightly packed
#pragma pack(push, 4)

// DDS Pixel Format structure
struct DDS_PIXELFORMAT {
    quint32 dwSize;
    quint32 dwFlags;
    quint32 dwFourCC;
    quint32 dwRGBBitCount;
    quint32 dwRBitMask;
    quint32 dwGBitMask;
    quint32 dwBBitMask;
    quint32 dwABitMask;
};

// DDS Header structure
struct DDS_HEADER {
    quint32 dwSize;
    quint32 dwFlags;
    quint32 dwHeight;
    quint32 dwWidth;
    quint32 dwPitchOrLinearSize;
    quint32 dwDepth;
    quint32 dwMipMapCount;
    quint32 dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    quint32 dwCaps;
    quint32 dwCaps2;
    quint32 dwCaps3;
    quint32 dwCaps4;
    quint32 dwReserved2[1];
};

#pragma pack(pop)

/***********************************************************************/

// DDS constants and flags
const quint32 DDS_MAGIC = 0x20534444; // "DDS "

// DDS_HEADER flags
const quint32 DDSD_CAPS = 0x1;
const quint32 DDSD_HEIGHT = 0x2;
const quint32 DDSD_WIDTH = 0x4;
const quint32 DDSD_PITCH = 0x8;
const quint32 DDSD_PIXELFORMAT = 0x1000;

// DDS_PIXELFORMAT flags
const quint32 DDPF_RGB = 0x40;
const quint32 DDPF_ALPHAPIXELS = 0x1;

// DDSCAPS flags
const quint32 DDSCAPS_COMPLEX = 0x8;
const quint32 DDSCAPS_TEXTURE = 0x1000;

// DDSCAPS2 flags
const quint32 DDSCAPS2_CUBEMAP = 0x200;
const quint32 DDSCAPS2_CUBEMAP_POSITIVEX = 0x400;
const quint32 DDSCAPS2_CUBEMAP_NEGATIVEX = 0x800;
const quint32 DDSCAPS2_CUBEMAP_POSITIVEY = 0x1000;
const quint32 DDSCAPS2_CUBEMAP_NEGATIVEY = 0x2000;
const quint32 DDSCAPS2_CUBEMAP_POSITIVEZ = 0x4000;
const quint32 DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x8000;
const quint32 DDSCAPS2_CUBEMAP_ALLFACES = (DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX |
                                            DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY |
                                            DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ);

// Clamp a value to the given range
template<typename T>
T clip(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
}

#endif // IMAGE_TO_CUBEMAP_HPP
