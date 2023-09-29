#include "pch.h"
#include "Pixel.h"

using namespace YunuD2D;

Pixel::Pixel(UINT32 pixel) :
    r((pixel & 0x00FF0000) >> 16),
    g((pixel & 0x0000FF00) >> 8),
    b((pixel & 0x000000FF)),
    a((pixel & 0xFF000000) >> 24)
{
}
Pixel::Pixel(BYTE r, BYTE g, BYTE b, BYTE a) :
    r(r),
    g(g),
    b(b),
    a(a)
{
}
