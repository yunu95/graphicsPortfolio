// YunuDX12Renderer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "YunuDX12Renderer.h"

// windows runtime template library
using namespace Microsoft::WRL;

#define MAX_LOADSTRING 100
constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, HWND* hWndOut);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// device가 모니터에 해당한다는데?
// main device represents virtual adapter. whatever that means....
// main device is used to create command allocators, command lists, command queues, fences, resources. whatever those mean...
ID3D12Device8* main_device = nullptr;
// idxgifactory is used for generating dxgi objects
IDXGIFactory7* dxgi_factory = nullptr;

IDXGIAdapter4* determine_main_adapter(IDXGIFactory7* factory)
{
    IDXGIAdapter4* adapter = nullptr;

    // get adapters in descending order of performance
    // enumerates graphics adapters based on preference which is passed as an argument
    for (UINT32 i = 0;
        factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) != DXGI_ERROR_NOT_FOUND;
        i++)
    {
        // 그래픽스 어댑터를 매개변수로 받아,디바이스를 생성할 수 있는지 본다.
        // 가장 마지막 변수는 그래픽 디바이스 포인터의 주소인데, 이걸 nullptr로 지정해 전달하면 디바이스가 생성가능한지 확인만 하고,
        // 막상 디바이스를 생성하지는 않는다.
        if (SUCCEEDED(D3D12CreateDevice(adapter, minimum_feature_level, __uuidof(ID3D12Device), nullptr)))
        {
            return adapter;
        }
        adapter->Release();
    }
    return nullptr;
}
D3D_FEATURE_LEVEL get_max_feature_level(IDXGIAdapter4* adapter)
{
    constexpr D3D_FEATURE_LEVEL feature_levels[4]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1,
    };

    D3D12_FEATURE_DATA_FEATURE_LEVELS feature_level_info{};
    feature_level_info.NumFeatureLevels = _countof(feature_levels);
    feature_level_info.pFeatureLevelsRequested = feature_levels;

    // ComPtr is like a smart pointer, so it's not really necessary to release device after its use.
    ComPtr<ID3D12Device> device;
    // create device only to check feature levels that are supported by its adapter
    DXCall(D3D12CreateDevice(adapter, minimum_feature_level, IID_PPV_ARGS(&device)));
    // check featuresupport of features that were requested in featre_level_info.
    // ? but sizeof? doesn't sizeof determine the size of something in compile time? what's it for?
    // ! there are various types of feature_level strcuctrues with unique sizes.
    DXCall(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_level_info, sizeof(feature_level_info)));
    return feature_level_info.MaxSupportedFeatureLevel;
}

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
    LoadStringW(hInstance, IDC_YUNUDX12RENDERER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    HWND hwnd;
    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow, &hwnd))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_YUNUDX12RENDERER));

    MSG msg;
    // d3d12 initialization
    constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

    //if (main_device)
    UINT32 dxgi_factory_flags = 0;

    // in case it's being compiled in debug mode, add factory debug flag for creating factory.
#ifdef _DEBUG
    // Enable debugging layer. Requires "graphics tools" optional feature
    {
        ComPtr<ID3D12Debug3> debug_interface;
        DXCall(D3D12GetDebugInterface(IID_PPV_ARGS(debug_interface.GetAddressOf())));
        debug_interface->EnableDebugLayer();
        dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    HRESULT hr;
    DXCall(hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory)));



    // ComPtr is a smart pointer, working similar to shared_ptr
    ComPtr<IDXGIAdapter4> main_adapter;
    // Attach method attaches primitive pointer to ComPtr
    main_adapter.Attach(determine_main_adapter(dxgi_factory));
    D3D_FEATURE_LEVEL max_feature_level = get_max_feature_level(main_adapter.Get());
    assert(max_feature_level >= minimum_feature_level);
    DXCall(D3D12CreateDevice(main_adapter.Get(), max_feature_level, IID_PPV_ARGS(&main_device)));

    // it provides the methods for submitting command lists.
    // it also handles how to synchronize commands execution, and how to update resource tile mappings
    ComPtr<ID3D12CommandQueue> commandQueue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    // type specifies the type of command list that the queue will be used for.
    // direct means it it used for rendering graphics directly.
    // bundle means it is used to record commands for part of a larger system.
    // compute means it is used for general purposes.
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    // priority among command queues.
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    // used for all sorts of thaings
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    // specifies something about node, which, I have no idea of.
    queueDesc.NodeMask = 0;

    ComPtr<ID3D12CommandAllocator> commandAllocator;

    main_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
    main_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocator.GetAddressOf()));

    // command list is issues rendering commands to the gpu.
    ComPtr<ID3D12GraphicsCommandList> commandList;


    // create the graphics command list
    DXCall(main_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        commandAllocator.Get(),
        nullptr,
        IID_PPV_ARGS(commandList.GetAddressOf())
    ));

    // get the width and height of the window
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    UINT hwndWidth = clientRect.right - clientRect.left;
    UINT hwndHeight = clientRect.bottom - clientRect.top;
    // create the swap chain
    IDXGISwapChain1* swapChain = nullptr;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
    swapChainDesc.Width = hwndWidth;
    swapChainDesc.Height = hwndHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // pixel format
    swapChainDesc.Stereo = FALSE;
    // sampledesc is for setting up multisampling.
    swapChainDesc.SampleDesc.Count = 1; // number of multisamples per pixel.
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // the intended usage of the back buffer.
    swapChainDesc.BufferCount = 2; // number of back buffers, typically 2.
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // how the back buffer is scaled.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // specifies whether the buffer contains alpha information.
    swapChainDesc.Flags = 0; // additional flags
    DXCall(dxgi_factory->CreateSwapChainForHwnd(commandQueue.Get(), hwnd, &swapChainDesc, nullptr, nullptr, &swapChain));
    ID3D12Resource* pBackBuffer = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
    ID3D12DescriptorHeap* rtvHeap;
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 2; // number of back buffers in swap chain.
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(hwndWidth);
    viewport.Height = static_cast<float>(hwndHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    // create descriptor heap, that contains all kinds of descriptors that describe important resources.
    main_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));

    // get the size of rtv descrip

    // get a pointer to the back buffer surface
    swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

    // create a descriptor handle for the rtv
    rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

    main_device->CreateRenderTargetView(pBackBuffer, nullptr, rtvHandle);

    // Set the render target to the first back buffer in the swap chain.
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear the render target to red.
    const float clearColor[] = { 1.0f, 0.3f, 0.3f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    // Get the descriptor for the resource
    D3D12_RESOURCE_DESC resourceDesc = pBackBuffer->GetDesc();
    // using this macro call writes debug string, making it easier to trace instantiations of com objects.
    NAME_D3D12_OBJECT(main_device, L"MAIN d3d12 DEVICE");
#ifdef _DEBUG
    {
        ComPtr<ID3D12InfoQueue> info_queue;
        DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
    }
#endif

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // graphics function
        commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        commandList->RSSetViewports(1, &viewport);
        swapChain->Present(1, 0);
        // graphics end
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
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
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_YUNUDX12RENDERER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_YUNUDX12RENDERER);
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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND* hWndOut)
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

    if (hWndOut)
        *hWndOut = hWnd;

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

void shutdown()
{
    dxgi_factory->Release();
#ifdef _DEBUG
    {
        ComPtr<ID3D12InfoQueue> info_queue;
        DXCall(main_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);

        ComPtr<ID3D12DebugDevice2> debug_device;
        DXCall(main_device->QueryInterface(IID_PPV_ARGS(&debug_device)));
        main_device->Release();
        DXCall(debug_device->ReportLiveDeviceObjects(
            D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));
    }
#endif
    main_device->Release();
}
