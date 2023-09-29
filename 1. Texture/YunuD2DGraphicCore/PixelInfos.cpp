#include "pch.h"
#include "PixelInfos.h"

using namespace YunuD2D;

PixelInfos::PixelInfos(IWICBitmap* pBitmap)
{
    UINT width, height;
    pBitmap->GetSize(&width, &height);
    IWICBitmapLock* pLock = nullptr;
    WICRect rc = { 0,0,(int)width,(int)height };
    this->width = width;
    this->height = height;
    HRESULT hr = pBitmap->Lock(&rc, WICBitmapLockRead, &pLock);

    WICPixelFormatGUID format;
    pBitmap->GetPixelFormat(&format);
    UINT cbBufferSize;
    BYTE* pBuffer = nullptr;
    pLock->GetDataPointer(&cbBufferSize, &pBuffer);

    int bytesPerPixel = cbBufferSize;
    bytesPerPixel /= width;
    bytesPerPixel /= height;

    for (UINT y = 0; y < height; y++)
    {
        pixels.push_back(vector<Pixel>());
        for (UINT x = 0; x < width; x++)
            if (bytesPerPixel == 4)
                pixels[y].push_back(Pixel(
                    pBuffer[y * width * 4 + x * 4 + 2],
                    pBuffer[y * width * 4 + x * 4 + 1],
                    pBuffer[y * width * 4 + x * 4],
                    pBuffer[y * width * 4 + x * 4 + 3]
                ));
            else
                pixels[y].push_back(Pixel(
                    pBuffer[y * width * 3 + x * 3 + 2],
                    pBuffer[y * width * 3 + x * 3 + 1],
                    pBuffer[y * width * 3 + x * 3],
                    0));
    }
    pLock->Release();
}
