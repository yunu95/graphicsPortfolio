// WinMain.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

//#define _CRTDB_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"
#include "GameProcess.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 게임 진행 클래스
    // 내부에서 윈도 생성과 메시지 콜백을 처리한다.
    // 또한 그래픽스 엔진 등을 포함한다.
    GameProcess* pGameProcess = new GameProcess();
    pGameProcess->Initialize(hInstance);
    pGameProcess->Loop();
    pGameProcess->Finalize();
    delete pGameProcess;
}

