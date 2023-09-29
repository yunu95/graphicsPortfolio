#pragma once
#include "SingletonClass.h"
#include "PixelInfos.h"
#include <d2d1.h>
#include <d2d1_1.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#ifdef YUNUD2DGRAPHICCORE_EXPORTS
#define YUNUD2DGRAPHICCORE_API __declspec(dllexport)
#else
#define YUNUD2DGRAPHICCORE_API __declspec(dllimport)
#endif

template <>
struct std::hash<D2D1::ColorF>
{
    std::size_t operator()(const D2D1::ColorF& k) const
    {
        using std::size_t;
        using std::hash;
        using std::string;

        return ((hash<float>()(k.r)
            ^ (hash<float>()(k.g))
            ^ (hash<float>()(k.b))
            ^ (hash<float>()(k.a))));
    }
};
bool operator==(const D2D1::ColorF& a, const D2D1::ColorF& b);

using namespace std;
namespace YunuD2D
{
    // This class is exported from the dll
    class YUNUD2DGRAPHICCORE_API YunuD2DGraphicCore : public SingletonClass<YunuD2DGraphicCore>
    {
    private:
        HWND hWnd = 0;
        ID2D1Factory* factory = nullptr;
        IDWriteFactory* writeFactory = nullptr;
        IWICImagingFactory* wicFactory = nullptr;
        ID2D1HwndRenderTarget* renderTarget = nullptr;
        IDWriteTextFormat* standardTextFormat = nullptr;
        HRESULT CreateDeviceResources();
        void DrawLine(const D2D1_POINT_2F& start, const D2D1_POINT_2F& end, float width, D2D1::ColorF color = D2D1::ColorF(0x000000, 1));

        ID2D1SolidColorBrush* brush;
        std::unordered_map<D2D1::ColorF, ID2D1SolidColorBrush*> cachedBrushes;
        std::unordered_map<wstring, ID2D1Bitmap*> cachedBitmaps;
        std::unordered_map<wstring, IWICBitmap*> cachedWicBitmaps;
        //std::map<D2D1::ColorF, ID2D1SolidColorBrush*> cachedBrushes;
        ID2D1SolidColorBrush* GetSolidColorBrush(D2D1::ColorF color);
        ID2D1Bitmap* LoadSprite(wstring filePath);
        IWICBitmap* LoadWicBitmap(wstring filePath);
    public:
        D2D1::ColorF backgroundColor = D2D1::ColorF::Black;
        HRESULT Initialize(HWND hWnd);
        YunuD2DGraphicCore(void);
        YunuD2DGraphicCore(YunuD2DGraphicCore&) = delete;
        YunuD2DGraphicCore& operator=(YunuD2DGraphicCore&) = delete;
        virtual ~YunuD2DGraphicCore();
        D2D1_SIZE_F GetRenderSize();

        void ResizeResolution(int width, int height);

        // Circle is a subset of ellipse
        void DrawCircle(const D2D1::Matrix3x2F& transform, const float radius, float drawWidth, D2D1::ColorF color = D2D1::ColorF(0x000000, 1), bool filled = false);
        // Rect is a subset of Polygon
        void DrawRect(float width, float height, float drawWidth, const D2D1::Matrix3x2F& transform, D2D1::ColorF color = D2D1::ColorF(0x000000, 1), bool filled = false);
        void DrawEllipse(float radiusX, float radiusY, float drawWidth, const D2D1::Matrix3x2F& transform, D2D1::ColorF color = D2D1::ColorF(0x000000, 1), bool filled = false);
        // Drawing polygon is dependent on drawing line
        void DrawPolygon(const std::vector<D2D1_POINT_2F> points, float drawWidth, const D2D1::Matrix3x2F& transform, D2D1::ColorF color = D2D1::ColorF(0x000000, 1), bool filled = false);
        void DrawTextImage(wstring str, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, double fontSize = 10, double width = 100, double height = 100);
        void DrawSprite(wstring filePath, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, double width = 100, double height = 100);
        PixelInfos GetPixelInfos(wstring imgFilepath);
        //void GetSpriteSize(wstring filePath,double* width,double* height);
        tuple<double, double> GetSpriteSize(wstring filePath);

            void BeginDraw();
        void EndDraw();
    };
}
