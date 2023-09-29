#pragma once

#ifdef YUNUD2DGRAPHICCORE_EXPORTS
#define YUNUD2DGRAPHICCORE_API __declspec(dllexport)
#else
#define YUNUD2DGRAPHICCORE_API __declspec(dllimport)
#endif

namespace YunuD2D
{
    struct YUNUD2DGRAPHICCORE_API Pixel
    {
        Pixel(UINT32 pixel);
        Pixel(BYTE r, BYTE g, BYTE b, BYTE a);
        unsigned short int r;
        unsigned short int g;
        unsigned short int b;
        unsigned short int a;
    };
};

