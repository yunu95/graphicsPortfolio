#pragma once
#include <windows.h>
#include "IYunu3DRenderer.h"
#include "GameTimer.h"

class I3DRenderer;

///
/// ������ ���� �����
/// �׻� ����� ���� ���μ���
///
/// [2020.4.3 LeHideHome]

class GameProcess
{
private:
	// �������� ����


	// ���� �ʱ� ��ġ
	const int m_WindowPosX = 300;
	const int m_WindowPosY = 300;

	// ȭ���� ũ��
	const int m_ScreenWidth = 1024;
	const int m_ScreenHeight = 768;

	// ���� ����
	HWND m_hWnd;
	MSG m_Msg;

	// 3D �׷��Ƚ� ����
	IYunu3DRenderer* m_pRenderer;
	static IYunu3DRenderer* m_pDREngineInstance;

	// �ð� ����
	GameTimer* m_pTimer;

private:
	// for Initialize
	void RecalcWindowSize();

	// for Loop
	void UpdateAll();
	void RenderAll();


public:
	// ������ �ʱ�ȭ�Ѵ�.
	HRESULT Initialize(HINSTANCE hInstance);

	// ���� ���� ����. ���� ������ ����Ǵ� �ݺ�����
	void Loop();

	// ������ �����Ѵ�.
	void Finalize();


	// �޽��� �ڵ鷯 (���� �ݹ�)
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

