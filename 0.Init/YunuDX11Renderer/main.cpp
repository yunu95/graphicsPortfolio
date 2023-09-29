// YunuDX11Renderer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "YunuDX11Renderer.h"

#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"Dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Dwrite.lib")
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d11_4.h>

#include <assert.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND* out_hwnd = nullptr);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_YUNUDX11RENDERER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    HWND hwnd;
    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow, &hwnd))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YUNUDX11RENDERER));

    MSG msg;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    ID3D11Device* d3dDevice;
    ID3D11DeviceContext* d3dImmediateContext;
    HRESULT hr = D3D11CreateDevice(
        0, // 기본 그래픽 어댑터
        D3D_DRIVER_TYPE_HARDWARE, // 참조 기기 따위를 쓰지 않음
        0,  // 
        createDeviceFlags,// 디버그 모드에서 시행시 디바이스 디버그 플래그를 킨다.
        0, 0, // 기능 수준체크는 기본으로, 즉 가장 최신부터 점검한다. 
        D3D11_SDK_VERSION,
        &d3dDevice,
        &featureLevel,
        &d3dImmediateContext
    );

    if (FAILED(hr))
    {
        MessageBox(0, L"d3d11 createdevice 부터 문제가 생겼다해.", 0, 0);
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        MessageBox(0, L"하드웨어가 direct 11을 지원하지 않는다 해.", 0, 0);
        return false;
    }
    UINT msaaQulaity;
    hr = d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQulaity);
    assert(msaaQulaity > 0);

    DXGI_SWAP_CHAIN_DESC chainDescription;
    // 블록체인의 사이즈를 0으로 하면 런타임중 hwnd를 지정하는 시점에서 가로세로 길이를 가져온다.
    chainDescription.BufferDesc.Width = 1920;
    chainDescription.BufferDesc.Height = 1080;
    // 60은 60fps를 가리키는 것이겠지만, denominator는 뭔고? 
    chainDescription.BufferDesc.RefreshRate.Numerator = 60;
    chainDescription.BufferDesc.RefreshRate.Denominator = 1;
    chainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    chainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    chainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // 계단현상 방지를 위한 msaa초기화코드
    chainDescription.SampleDesc.Count = 4;
    chainDescription.SampleDesc.Quality = msaaQulaity - 1;

    chainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    chainDescription.BufferCount = 1;
    chainDescription.OutputWindow = hwnd;
    chainDescription.Windowed = true;
    //chainDescription.Stereo = false;
    //chainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    chainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    chainDescription.Flags = 0;

    // d3dDevice에게 질의해 dxgi device를 가져온다.
    // dxgi : directX graphics infrastructure
    IDXGIDevice* dxgiDevice = 0;
    d3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

    // adapter
    IDXGIAdapter* dxgiAdapter = 0;
    dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);

    IDXGIFactory* dxgiFactory = 0;
    dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);

    IDXGISwapChain* swapChain;
    dxgiFactory->CreateSwapChain(d3dDevice, &chainDescription, &swapChain);

    dxgiDevice->Release();
    dxgiAdapter->Release();
    dxgiFactory->Release();

    ID3D11RenderTargetView* renderTargetView;
    ID3D11Texture2D* backBuffer;
    swapChain->GetBuffer(0,         // buffer index : in case of there are more than one back buffer.
        __uuidof(ID3D11Texture2D),  // 버퍼의 인터페이스 형식은 2d 텍스처 
        reinterpret_cast<void**>(&backBuffer) // 후면 버퍼를 가리키는 포인터
    );
    d3dDevice->CreateRenderTargetView(
        backBuffer,         // 렌더 대상으로 사용할 자원
        0,
        &renderTargetView
    );
    backBuffer->Release();

    D3D11_TEXTURE2D_DESC depthStencilDesc; // 픽셀의 깊이값을 저장하는 뎁스-스텐실 텍스처, 일반 텍스처와 자료형은 같다.
    depthStencilDesc.Width = 1920;
    depthStencilDesc.Height = 1080;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // 스왑체인의 msaa 설정과 뎁스스텐실의 설정값은 서로 일치해야 한다.
    depthStencilDesc.SampleDesc.Count = 4;
    depthStencilDesc.SampleDesc.Quality = msaaQulaity - 1; // msaaQuality-1은 무슨 의미인가?

    // 이 자원을 GPU가 읽는다는 뜻이다.
    // 이 외의 옵션으로는 immutable(gpu 읽기만 가능), dynamic (cpu가 자원을 갱신가능),
    // staging(자원을 비디오 메모리에서 시스템 메모리로 전송할 수 있음)이 있다.  
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 이 텍스처로 말할것 같으면, 바인딩 될 때 뎁스-스텐실로 적용되는 텍스처이올시다.
    // cpu가 자원을 읽는 방식을 결정한다. 위의 Usage가 호환되어야 한다.
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    ID3D11Texture2D* depthStencilBuffer; // 위에서 정의한 건 텍스처의 명세, 이건 2d 텍스처 그 자체
    ID3D11DepthStencilView* depthStencilView;

    d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);
    d3dDevice->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);

    constexpr float blue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    constexpr float red[] = { 0.75f, 0.0f, 0.0f, 1.0f };

    d3dImmediateContext->ClearRenderTargetView(renderTargetView, red);
    hr = swapChain->Present(0, 0);
    d3dImmediateContext->ClearRenderTargetView(renderTargetView, blue);
    // OM : output merger, 출력 병합기
    ////d3dImmediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    // 백버퍼의 설정값을 가져온다.
    D3D11_TEXTURE2D_DESC backBufferDesc = {};
    backBuffer->GetDesc(&backBufferDesc);
    // 가져 온 백버퍼 설정값으로 뷰포트를 초기화
    CD3D11_VIEWPORT viewPort(
        0.f,
        0.f,
        static_cast<float>(backBufferDesc.Width),
        static_cast<float>(backBufferDesc.Height)
    );
    //m_viewPort = viewPort;
    // 디바이스 컨텍스트에 뷰포트를 연결한다
    d3dImmediateContext->RSSetViewports(1, &viewPort);
    // Main message loop:
    while (true)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            switch (msg.message)
            {
            case WM_QUIT:
                return 0;
            }
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        //constexpr float blue[] = { 0,0,1.0f,1.0f };
        //constexpr float red[] = { 1,0,1.0f,1.0f };

        //d3dImmediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
        //d3dImmediateContext->ClearRenderTargetView(renderTargetView, blue);
        //static int index = 0;
        //index = ++index % 2;
        //constexpr const float* colors[] = {red,blue};
        //d3dImmediateContext->ClearRenderTargetView(renderTargetView, blue);
        //d3dImmediateContext->ClearRenderTargetView(renderTargetView, red);
        d3dImmediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
        d3dImmediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
        hr = swapChain->Present(0, 0);
    }

    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YUNUDX11RENDERER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_YUNUDX11RENDERER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND* out_hwnd)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    if (out_hwnd)
        *out_hwnd = hWnd;

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
