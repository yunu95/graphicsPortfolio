#pragma once
#include <windows.h>
#include "IYunu3DRenderer.h"
#include "GameTimer.h"

class I3DRenderer;

///
/// 간단한 엔진 데모용
/// 항상 만들던 게임 프로세스
///
/// [2020.4.3 LeHideHome]

class GameProcess
{
private:
	// 공통적인 설정


	// 윈도 초기 위치
	const int m_WindowPosX = 300;
	const int m_WindowPosY = 300;

	// 화면의 크기
	const int m_ScreenWidth = 1024;
	const int m_ScreenHeight = 768;

	// 윈도 관련
	HWND m_hWnd;
	MSG m_Msg;

	// 3D 그래픽스 엔진
	IYunu3DRenderer* m_pRenderer;
	static IYunu3DRenderer* m_pDREngineInstance;

	// 시간 제어
	GameTimer* m_pTimer;

private:
	// for Initialize
	void RecalcWindowSize();

	// for Loop
	void UpdateAll();
	void RenderAll();


public:
	// 게임을 초기화한다.
	HRESULT Initialize(HINSTANCE hInstance);

	// 메인 게임 루프. 실제 게임이 진행되는 반복구문
	void Loop();

	// 게임을 정리한다.
	void Finalize();


	// 메시지 핸들러 (윈도 콜백)
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

