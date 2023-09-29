#pragma once

#include <Windows.h>

class Yunu3DRenderer
{
public:
    //Yunu3DRenderer(HWND hWnd, int screenWidth, int screenHeight);
    virtual ~Yunu3DRenderer() {}

    // DX�� �ʱ�ȭ�Ѵ�.
    virtual bool Initialize(HWND hWnd) = 0;
    // ������ ����
    virtual void BeginRender() = 0;
    virtual void Render() = 0;
    // ������ ������. �׸� �� �� �׸� ���¿��� Present
    virtual void EndRender() = 0;
    // ������ �����Ѵ�.
    virtual void Release() = 0;
protected:
    Yunu3DRenderer() {};
    // â ũ�Ⱑ ������ �� ó���ؾ� �� �͵�
    virtual void OnResize() = 0;
};

