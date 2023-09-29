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
        0, // �⺻ �׷��� �����
        D3D_DRIVER_TYPE_HARDWARE, // ���� ��� ������ ���� ����
        0,  // 
        createDeviceFlags,// ����� ��忡�� ����� ����̽� ����� �÷��׸� Ų��.
        0, 0, // ��� ����üũ�� �⺻����, �� ���� �ֽź��� �����Ѵ�. 
        D3D11_SDK_VERSION,
        &d3dDevice,
        &featureLevel,
        &d3dImmediateContext
    );

    if (FAILED(hr))
    {
        MessageBox(0, L"d3d11 createdevice ���� ������ �������.", 0, 0);
        return false;
    }

    if (featureLevel != D3D_FEATURE_LEVEL_11_0)
    {
        MessageBox(0, L"�ϵ��� direct 11�� �������� �ʴ´� ��.", 0, 0);
        return false;
    }
    UINT msaaQulaity;
    hr = d3dDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQulaity);
    assert(msaaQulaity > 0);

    DXGI_SWAP_CHAIN_DESC chainDescription;
    // ���ü���� ����� 0���� �ϸ� ��Ÿ���� hwnd�� �����ϴ� �������� ���μ��� ���̸� �����´�.
    chainDescription.BufferDesc.Width = 1920;
    chainDescription.BufferDesc.Height = 1080;
    // 60�� 60fps�� ����Ű�� ���̰�����, denominator�� ����? 
    chainDescription.BufferDesc.RefreshRate.Numerator = 60;
    chainDescription.BufferDesc.RefreshRate.Denominator = 1;
    chainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    chainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    chainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    // ������� ������ ���� msaa�ʱ�ȭ�ڵ�
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

    // d3dDevice���� ������ dxgi device�� �����´�.
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
        __uuidof(ID3D11Texture2D),  // ������ �������̽� ������ 2d �ؽ�ó 
        reinterpret_cast<void**>(&backBuffer) // �ĸ� ���۸� ����Ű�� ������
    );
    d3dDevice->CreateRenderTargetView(
        backBuffer,         // ���� ������� ����� �ڿ�
        0,
        &renderTargetView
    );
    backBuffer->Release();

    D3D11_TEXTURE2D_DESC depthStencilDesc; // �ȼ��� ���̰��� �����ϴ� ����-���ٽ� �ؽ�ó, �Ϲ� �ؽ�ó�� �ڷ����� ����.
    depthStencilDesc.Width = 1920;
    depthStencilDesc.Height = 1080;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // ����ü���� msaa ������ �������ٽ��� �������� ���� ��ġ�ؾ� �Ѵ�.
    depthStencilDesc.SampleDesc.Count = 4;
    depthStencilDesc.SampleDesc.Quality = msaaQulaity - 1; // msaaQuality-1�� ���� �ǹ��ΰ�?

    // �� �ڿ��� GPU�� �д´ٴ� ���̴�.
    // �� ���� �ɼ����δ� immutable(gpu �б⸸ ����), dynamic (cpu�� �ڿ��� ���Ű���),
    // staging(�ڿ��� ���� �޸𸮿��� �ý��� �޸𸮷� ������ �� ����)�� �ִ�.  
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // �� �ؽ�ó�� ���Ұ� ������, ���ε� �� �� ����-���ٽǷ� ����Ǵ� �ؽ�ó�̿ýô�.
    // cpu�� �ڿ��� �д� ����� �����Ѵ�. ���� Usage�� ȣȯ�Ǿ�� �Ѵ�.
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    ID3D11Texture2D* depthStencilBuffer; // ������ ������ �� �ؽ�ó�� ��, �̰� 2d �ؽ�ó �� ��ü
    ID3D11DepthStencilView* depthStencilView;

    d3dDevice->CreateTexture2D(&depthStencilDesc, 0, &depthStencilBuffer);
    d3dDevice->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);

    constexpr float blue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
    constexpr float red[] = { 0.75f, 0.0f, 0.0f, 1.0f };

    d3dImmediateContext->ClearRenderTargetView(renderTargetView, red);
    hr = swapChain->Present(0, 0);
    d3dImmediateContext->ClearRenderTargetView(renderTargetView, blue);
    // OM : output merger, ��� ���ձ�
    ////d3dImmediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

    // ������� �������� �����´�.
    D3D11_TEXTURE2D_DESC backBufferDesc = {};
    backBuffer->GetDesc(&backBufferDesc);
    // ���� �� ����� ���������� ����Ʈ�� �ʱ�ȭ
    CD3D11_VIEWPORT viewPort(
        0.f,
        0.f,
        static_cast<float>(backBufferDesc.Width),
        static_cast<float>(backBufferDesc.Height)
    );
    //m_viewPort = viewPort;
    // ����̽� ���ؽ�Ʈ�� ����Ʈ�� �����Ѵ�
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
