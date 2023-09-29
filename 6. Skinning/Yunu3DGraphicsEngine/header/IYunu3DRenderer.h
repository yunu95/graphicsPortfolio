#pragma once

#include <Windows.h>

class IYunu3DRenderer
{
public:
    IYunu3DRenderer() {};
    virtual ~IYunu3DRenderer() {}

    // DX를 초기화한다.
    virtual bool Initialize(HWND hWnd) = 0;

    // 렌더링 시작
    virtual void BeginRender() = 0;
    virtual void Render() = 0;

    // 렌더링 마무리. 그릴 것 다 그린 상태에서 Present
    virtual void EndRender() = 0;

    // 엔진을 종료한다.
    virtual void Release() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void OnMouseDown(int x, int y) = 0;
    virtual void OnMouseUp(int x, int y) = 0;
    virtual void OnMouseMove(int btnState, int x, int y) = 0;

    // 창 크기가 변했을 때 처리해야 할 것들
    virtual void OnResize() = 0;
protected:
};

