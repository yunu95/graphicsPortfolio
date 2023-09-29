#pragma once
#include <d2d1.h>
#include <vector>
#include "Pixel.h"

#ifdef YUNUD2DGRAPHICCORE_EXPORTS
#define YUNUD2DGRAPHICCORE_API __declspec(dllexport)
#else
#define YUNUD2DGRAPHICCORE_API __declspec(dllimport)
#endif

using namespace std;

namespace YunuD2D
{
    class YunuD2DGraphicCore;
    struct YUNUD2DGRAPHICCORE_API PixelInfos
    {
        int width;
        int height;
        vector<vector<Pixel>> pixels;
        virtual ~PixelInfos() {}
    private:
        PixelInfos(IWICBitmap* pBitmap);
        friend YunuD2DGraphicCore;
    };
}
