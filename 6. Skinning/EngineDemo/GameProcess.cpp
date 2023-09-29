#include "GameProcess.h"

//#define _DRSOFTRENDERER_
//#define _DRDX11RENDERER_
#include "IYunu3DRenderer.h"

#define _YunuDX11RENDERER_

#ifdef _DRDX11RENDERER_
#include "../DRSoftRenderer/DRSoftRenderer.h"
#endif

#ifdef _YunuDX11RENDERER_
#include "../YunuDX11Renderer/YunuDX11Renderer.h"
#endif

IYunu3DRenderer* GameProcess::m_pYunu3DEngineInstance = nullptr;


HRESULT GameProcess::Initialize(HINSTANCE hInstance)
{
    /// Win32 ����
    // ���� Ŭ����
    wchar_t szAppName[] = L"GameAcademy 2021 3D Engine Demo";
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = GameProcess::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szAppName;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);


    // ���� Ŭ���� ���
    RegisterClassExW(&wcex);

    // ���� ����
    m_hWnd = CreateWindowW(szAppName, szAppName,
        WS_OVERLAPPEDWINDOW,
        m_WindowPosX, m_WindowPosY, m_ScreenWidth, m_ScreenHeight,
        nullptr, nullptr, hInstance, NULL);

    if (!m_hWnd) return E_INVALIDARG;


    /// �׷��Ƚ� ���� ����
    // �׷��Ƚ� ���� �ʱ�ȭ
#ifdef _DRDX11RENDERER_
    m_pRenderer = new DRDX11Renderer();
#else
    m_pRenderer = new YunuDX11Renderer();
#endif
    m_pRenderer->Initialize(m_hWnd);

    m_pYunu3DEngineInstance = m_pRenderer;

    // ������ ������ ȭ�鿡 ǥ��
    ShowWindow(m_hWnd, SW_SHOWNORMAL);
    UpdateWindow(m_hWnd);

    // Ŭ���̾�Ʈ ������ ���ϴ� ���� �ػ󵵰� �ǵ��� ���� ������ ������
    // 21���⿡ �� �̷��� �ϴ°� �ʹ�.
    RecalcWindowSize();

    m_pTimer = new GameTimer();
    m_pTimer->Reset();

    return S_OK;
}

void GameProcess::Loop()
{
    while (true)
    {
        if (PeekMessage(&m_Msg, NULL, 0, 0, PM_REMOVE))
        {
            if (m_Msg.message == WM_QUIT) break;

            DispatchMessage(&m_Msg);
        }
        if (PeekMessage(&m_Msg, NULL, 0, 0, WM_SIZE))
        {
            m_pRenderer->OnResize();
        }
        else
        {
            UpdateAll();
            RenderAll();
        }
    }
}

// â ����� ���, Ŭ���̾�Ʈ ������ ũ��� ���� ũ���� ���̸� �����Ѵ�.
// ��� 16�������� �׳� ����� ������ ���������.
// �׷��� ���� ��Ÿ�ϰ� �������� �� ����� ��� ���Ѵ�..
void GameProcess::RecalcWindowSize()
{
    RECT nowRect;
    DWORD _style = (DWORD)GetWindowLong(m_hWnd, GWL_STYLE);
    DWORD _exstyle = (DWORD)GetWindowLong(m_hWnd, GWL_EXSTYLE);

    GetWindowRect(m_hWnd, &nowRect);

    RECT newRect;
    newRect.left = 0;
    newRect.top = 0;
    newRect.right = m_ScreenWidth;
    newRect.bottom = m_ScreenHeight;

    AdjustWindowRectEx(&newRect, _style, NULL, _exstyle);

    // Ŭ���̾�Ʈ �������� ���� ũ��� �� Ŀ�� �Ѵ�. (�ܰ���, Ÿ��Ʋ ��)
    int _newWidth = (newRect.right - newRect.left);
    int _newHeight = (newRect.bottom - newRect.top);

    SetWindowPos(m_hWnd, HWND_NOTOPMOST, nowRect.left, nowRect.top,
        _newWidth, _newHeight, SWP_SHOWWINDOW);
}

void GameProcess::UpdateAll()
{
    // �� ������ �ð��� ����Ѵ�.
    m_pTimer->Tick();

    // �� �������� deltaTime
    m_pTimer->DeltaTime();

    // Ű �Է�
    if (GetAsyncKeyState(VK_RETURN))
    {

    }

    if (GetAsyncKeyState(VK_ESCAPE))
    {
        PostQuitMessage(0);
    }

    /// Ÿ�̸Ӹ� ������� FPS, millisecond per frame (1�����Ӵ� elapsedTime = deltaTime)�� ��� �� �ش�.

    // Code computes the average frames per second, and also the 
    // average time it takes to render one frame.  These stats 
    // are appended to the window caption bar.

    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_pTimer->TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        /*
        std::wostringstream outs;
        outs.precision(6);
        outs << mMainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(mhMainWnd, outs.str().c_str());
        */

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }


    /// ���� ������Ʈ
    m_pRenderer->Update(m_pTimer->DeltaTime());
}

void GameProcess::RenderAll()
{
    /// �׸��⸦ �غ��Ѵ�.
    m_pRenderer->BeginRender();

    /// �������� �׸��⸦ �Ѵ�.
    m_pRenderer->Render();

    /// �׸��⸦ ������.
    m_pRenderer->EndRender();
}

void GameProcess::Finalize()
{
    m_pRenderer->Release();
    delete m_pTimer;
    delete m_pRenderer;
}

// �޽��� �ڵ鷯 (���� �ݹ�)
LRESULT CALLBACK GameProcess::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC			hdc;
    PAINTSTRUCT ps;

    switch (message)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.  
        // We pause the game when the window is deactivated and unpause it 
        // when it becomes active.  
    case WM_ACTIVATE:
        /*
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            mAppPaused = true;
            mTimer.Stop();
        }
        else
        {
            mAppPaused = false;
            mTimer.Start();
        }
        */
        return 0;

        // WM_SIZE is sent when the user resizes the window.  
    case WM_SIZE:
        /*
        // Save the new client area dimensions.
        m_pRenderer->mClientWidth = LOWORD(lParam);
        m_pRenderer->mClientHeight = HIWORD(lParam);
        if (md3dDevice)
        {
            if (wParam == SIZE_MINIMIZED)
            {
                mAppPaused = true;
                mMinimized = true;
                mMaximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                mAppPaused = false;
                mMinimized = false;
                mMaximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                // Restoring from minimized state?
                if (mMinimized)
                {
                    mAppPaused = false;
                    mMinimized = false;
                    OnResize();
                }

                // Restoring from maximized state?
                else if (mMaximized)
                {
                    mAppPaused = false;
                    mMaximized = false;
                    OnResize();
                }
                else if (mResizing)
                {
                    // If user is dragging the resize bars, we do not resize
                    // the buffers here because as the user continuously
                    // drags the resize bars, a stream of WM_SIZE messages are
                    // sent to the window, and it would be pointless (and slow)
                    // to resize for each WM_SIZE message received from dragging
                    // the resize bars.  So instead, we reset after the user is
                    // done resizing the window and releases the resize bars, which
                    // sends a WM_EXITSIZEMOVE message.
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    OnResize();
                }
            }
        }
        */
        //m_pRenderer->
        GameProcess::m_pYunu3DEngineInstance->OnResize();
        return 0;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        // 			mAppPaused = true;
        // 			mResizing = true;
        // 			mTimer.Stop();
        return 0;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        // 			mAppPaused = false;
        // 			mResizing = false;
        // 			mTimer.Start();
        // 			OnResize();
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        m_pYunu3DEngineInstance->OnMouseDown(/*wParam, */LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        m_pYunu3DEngineInstance->OnMouseUp(/*wParam, */LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_MOUSEMOVE:
        m_pYunu3DEngineInstance->OnMouseMove(wParam, LOWORD(lParam), HIWORD(lParam));

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
