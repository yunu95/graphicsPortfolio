#pragma once

#include <Windows.h>

class IYunu3DRenderer
{
public:
    IYunu3DRenderer() {};
    virtual ~IYunu3DRenderer() {}

    // DX�� �ʱ�ȭ�Ѵ�.
    virtual bool Initialize(HWND hWnd) = 0;

    // ������ ����
    virtual void BeginRender() = 0;
    virtual void Render() = 0;

    // ������ ������. �׸� �� �� �׸� ���¿��� Present
    virtual void EndRender() = 0;

    // ������ �����Ѵ�.
    virtual void Release() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void OnMouseDown(int x, int y) = 0;
    virtual void OnMouseUp(int x, int y) = 0;
    virtual void OnMouseMove(int btnState, int x, int y) = 0;

    // â ũ�Ⱑ ������ �� ó���ؾ� �� �͵�
    virtual void OnResize() = 0;
protected:
};

