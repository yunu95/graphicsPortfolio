// YunuD2DGraphicCore.cpp : Defines the exported functions for the DLL.
//
#include "pch.h"
#include <vector>
#include <cassert>
#include <d2d1.h>

using namespace YunuD2D;

bool operator==(const D2D1::ColorF& a, const D2D1::ColorF& b)
{
    return a.a == b.a &&
        a.r == b.r &&
        a.g == b.g &&
        a.b == b.b;
}
// This is the constructor of a class that has been exported.
YunuD2D::YunuD2DGraphicCore::YunuD2DGraphicCore()
{
    return;
}

// 그래픽 코어 객체는 죽을 때 팩토리와 함께 죽는다.
YunuD2D::YunuD2DGraphicCore::~YunuD2DGraphicCore()
{
    if (factory != NULL)
        factory->Release();
    if (writeFactory != NULL)
        writeFactory->Release();
    if (wicFactory != NULL)
        wicFactory->Release();

    factory = NULL;
    writeFactory = NULL;
    wicFactory = NULL;
}
HRESULT YunuD2D::YunuD2DGraphicCore::Initialize(HWND hWnd)
{
    if (factory && renderTarget)
        return S_OK;

    this->hWnd = hWnd;
    HRESULT hr = S_OK;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);

    if (hr != S_OK)
        return hr;

    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&writeFactory));

    if (hr != S_OK)
        return hr;

    hr = CoInitialize(nullptr);
    if (FAILED(hr))
        return hr;

    hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wicFactory)
    );

    if (hr != S_OK)
        return hr;

    hr = writeFactory->CreateTextFormat(
        L"Verdana",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        50,
        L"",
        &standardTextFormat
    );

    if (hr != S_OK)
        return hr;

    hr = CreateDeviceResources();

    return hr;
}

HRESULT YunuD2D::YunuD2DGraphicCore::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (renderTarget == nullptr)
    {
        RECT rc;
        GetClientRect(hWnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top);

        hr = factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(hWnd, size),
            &renderTarget);
    }

    return hr;
}

ID2D1SolidColorBrush* YunuD2D::YunuD2DGraphicCore::GetSolidColorBrush(D2D1::ColorF color)
{
    if (cachedBrushes.find(color) == cachedBrushes.end())
    {
        ID2D1SolidColorBrush* brush;
        renderTarget->CreateSolidColorBrush(color, &brush);
        //cachedBrushes[color] = unique_ptr<ID2D1SolidColorBrush>(brush);
        cachedBrushes[color] = brush;
        //cachedBrushes[color] = brush;
    }
    return cachedBrushes[color];
    //return cachedBrushes[color];
}
D2D1_SIZE_F YunuD2D::YunuD2DGraphicCore::GetRenderSize()
{
    return renderTarget->GetSize();
}
void YunuD2D::YunuD2DGraphicCore::ResizeResolution(int width, int height)
{
    auto pixelSize = renderTarget->GetPixelSize();
    if (pixelSize.width == width && pixelSize.height == height)
        return;

    factory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(width, height)),
        &renderTarget);
    for (auto each : cachedBitmaps)
        if (each.second)
            each.second->Release();
    for (auto each : cachedWicBitmaps)
        if (each.second)
            each.second->Release();
    for (auto each : cachedBrushes)
        if (each.second)
            each.second->Release();
    cachedBitmaps.clear();
    cachedWicBitmaps.clear();
    cachedBrushes.clear();
}
void YunuD2D::YunuD2DGraphicCore::DrawRect(float width, float height, float drawWidth, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, bool filled)
{
    vector<D2D1_POINT_2F> points(4, D2D1_POINT_2F());
    points[0].x = -0.5 * width;
    points[1].x = -0.5 * width;
    points[2].x = 0.5 * width;
    points[3].x = 0.5 * width;

    points[0].y = -0.5 * height;
    points[1].y = 0.5 * height;
    points[2].y = 0.5 * height;
    points[3].y = -0.5 * height;

    for (D2D1_POINT_2F& each : points)
        each = transform.TransformPoint(each);

    DrawPolygon(points, drawWidth, D2D1::Matrix3x2F::Identity(), color, filled);
}
void YunuD2D::YunuD2DGraphicCore::DrawLine(const D2D1_POINT_2F& start, const D2D1_POINT_2F& end, float width, D2D1::ColorF color)
{
    renderTarget->DrawLine(start, end, GetSolidColorBrush(color), width);
}
void YunuD2D::YunuD2DGraphicCore::DrawCircle(const D2D1::Matrix3x2F& transform, const float radius, float drawWidth, D2D1::ColorF color, bool filled)
{
    DrawEllipse(radius, radius, drawWidth, transform, color, filled);
}
void YunuD2D::YunuD2DGraphicCore::DrawEllipse(float radiusX, float radiusY, float drawWidth, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, bool filled)
{
    D2D1_ELLIPSE ellipse;
    ellipse.radiusX = radiusX;
    ellipse.radiusY = radiusY;
    ellipse.point.x = 0;
    ellipse.point.y = 0;
    ID2D1SolidColorBrush* brush = GetSolidColorBrush(color);

    renderTarget->SetTransform(transform);
    if (filled)
        renderTarget->FillEllipse(ellipse, brush);
    renderTarget->DrawEllipse(ellipse, brush, drawWidth);
    renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}
void YunuD2D::YunuD2DGraphicCore::DrawTextImage(wstring str, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, double fontSize, double width, double height)
{
    if (width == 0 || height == 0 || fontSize == 0)
        return;

    ID2D1SolidColorBrush* brush = GetSolidColorBrush(color);
    D2D1_POINT_2F origin;
    IDWriteTextLayout* textLayout = nullptr;
    IDWriteTextFormat* textFormat;
    writeFactory->CreateTextFormat(
        L"Verdana",
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        &textFormat
    );
    writeFactory->CreateTextLayout(str.c_str(), str.length(), textFormat, width, height, &textLayout);
    textLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    textLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    origin.x = -0.5 * width;
    origin.y = -0.5 * height;

    renderTarget->SetTransform(transform);
    renderTarget->DrawTextLayout(origin, textLayout, brush);
    renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    textLayout->Release();
    textFormat->Release();
}
void YunuD2D::YunuD2DGraphicCore::DrawPolygon(const std::vector<D2D1_POINT_2F> points, float drawWidth, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, bool filled)
{
    ID2D1PathGeometry* geometry;
    ID2D1GeometrySink* pSink;
    factory->CreatePathGeometry(&geometry);

    geometry->Open(&pSink);
    pSink->BeginFigure(transform.TransformPoint(points[0]), D2D1_FIGURE_BEGIN_FILLED);
    for (int i = 1; i < points.size(); i++)
        pSink->AddLine(transform.TransformPoint(points[i]));
    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
    pSink->Close();
    pSink->Release();

    if (filled)
        renderTarget->FillGeometry(geometry, GetSolidColorBrush(color));
    renderTarget->DrawGeometry(geometry, GetSolidColorBrush(color), drawWidth);
    geometry->Release();
}

void YunuD2D::YunuD2DGraphicCore::BeginDraw()
{
    HRESULT hr = CreateDeviceResources();

    if (SUCCEEDED(hr) && !(renderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
    {
        // Retrieve the size of the render target.
        D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();
        renderTarget->BeginDraw();
        renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
        renderTarget->Clear(backgroundColor);
    }

}
void YunuD2D::YunuD2DGraphicCore::EndDraw()
{
    renderTarget->EndDraw();
}
ID2D1Bitmap* YunuD2D::YunuD2DGraphicCore::LoadSprite(wstring filePath)
{
    if (cachedBitmaps.find(filePath) != cachedBitmaps.end())
        return cachedBitmaps[filePath];

    // Load the sprite image from a file
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    HRESULT hr = wicFactory->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr))
        return nullptr;

    hr = decoder->GetFrame(0, &frame);
    hr = wicFactory->CreateFormatConverter(&converter);

    hr = converter->Initialize(
        frame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.f,
        WICBitmapPaletteTypeCustom
    );
    cachedBitmaps[filePath] = nullptr;
    hr = renderTarget->CreateBitmapFromWicBitmap(converter, nullptr, &cachedBitmaps[filePath]);

    return cachedBitmaps[filePath];
}
IWICBitmap* YunuD2D::YunuD2DGraphicCore::LoadWicBitmap(wstring filePath)
{
    if (cachedWicBitmaps.find(filePath) != cachedWicBitmaps.end())
        return cachedWicBitmaps[filePath];

    // Load the sprite image from a file
    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;
    HRESULT hr = wicFactory->CreateDecoderFromFilename(
        filePath.c_str(),
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &decoder
    );
    if (FAILED(hr))
        return nullptr;

    hr = decoder->GetFrame(0, &frame);
    cachedBitmaps[filePath] = nullptr;
    hr = wicFactory->CreateBitmapFromSource(frame, WICBitmapCacheOnLoad, &cachedWicBitmaps[filePath]);

    return cachedWicBitmaps[filePath];
}
tuple<double, double> YunuD2D::YunuD2DGraphicCore::GetSpriteSize(wstring filePath)
{
    auto sprite = LoadSprite(filePath);
    double width = 0;
    double height = 0;
    if (sprite != nullptr)
    {
        auto size = sprite->GetPixelSize();
        width = (double)size.width;
        height = (double)size.height;
    }
    return tie(width, height);
}
void YunuD2D::YunuD2DGraphicCore::DrawSprite(wstring filePath, const D2D1::Matrix3x2F& transform, D2D1::ColorF color, double width, double height)
{
    auto sprite = LoadSprite(filePath);
    if (sprite == nullptr)
        return;


    D2D1_RECT_F rect;
    auto renderTargetSize = renderTarget->GetSize();
    /*rect.bottom = transform.dy + 0.5 * height * transform.m22;
    rect.top = transform.dy - 0.5 * height * transform.m22;
    rect.right = transform.dx + 0.5 * width * transform.m11;
    rect.left = transform.dx - 0.5 * width * transform.m11;*/

    rect.bottom = 0.5 * height;
    rect.top = -0.5 * height;
    rect.right = 0.5 * width;
    rect.left = -0.5 * width;
    if (rect.left >= renderTargetSize.width || rect.top >= renderTargetSize.height ||
        rect.right <= 0.0f || rect.bottom <= 0.0f)
        return;

    renderTarget->SetTransform(transform);
    renderTarget->DrawBitmap(sprite, rect, color.a);
    renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
}
PixelInfos YunuD2DGraphicCore::GetPixelInfos(wstring imgFilepath)
{
    return PixelInfos(LoadWicBitmap(imgFilepath));
}
