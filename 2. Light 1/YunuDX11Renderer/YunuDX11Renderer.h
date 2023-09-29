#pragma once

#include "IYunu3DRenderer.h"
#include <Windows.h>
#include "DebugSphere.h"
#include "DebugCone.h"
#include "DebugArrow.h"
//#include "GameTimer.h"		// Timer�ε� ���� �ܺο��� �޴´�. (���躯��)
#include "d3dUtil.h"

#include "d3dx11Effect.h"	// Box, Skull
#include "Vertex.h"	
#include "InputLayout.h"	


class DXTKFont;
class Camera;

class ASEParser;
namespace render_object
{
    namespace Helper
    {
        class Axis;
        class Grid;
    }
    class MeshObject;
    class Box;
    class Crate;
    //class SiegeTank;
}

#ifdef YunuDX11DLL_EXPORTS
#define YunuDX11API __declspec(dllexport)
#else
#define YunuDX11API __declspec(dllimport)
#endif
using namespace render_object;
using namespace render_object::Helper;

class YunuDX11API YunuDX11Renderer : public IYunu3DRenderer
{
public:
    YunuDX11Renderer();
    ~YunuDX11Renderer();

    // DX�� �ʱ�ȭ�Ѵ�.
    virtual bool Initialize(HWND hWnd);
    // ������ ����
    virtual void BeginRender();
    virtual void Render();
    // ������ ������. �׸� �� �� �׸� ���¿��� Present
    virtual void EndRender();
    // ������ �����Ѵ�.
    virtual void Release();


    virtual void Update(float deltaTime);
protected:
    // â ũ�Ⱑ ������ �� ó���ؾ� �� �͵�
    virtual void OnResize();
    // ȭ�鿡 ����� ���� ���
private:
    void Draw_Status();

    float GetAspectRatio() const;


    // �ð� ����
private:
    float m_deltaTime;


    /// DirectX 11 ���� ���

    // �ʱ�ȭ
private:
    //HINSTANCE mhAppInst;
    HWND      mhMainWnd;				/// ���� �ڵ�
    bool      mAppPaused;
    bool      mMinimized;
    bool      mMaximized;
    bool      mResizing;				/// �̷� �͵��� ���ؼ� Win32App���� ���� ���� Ŭ������ ����� �� ���������.
    UINT      m4xMsaaQuality;


    ID3D11Device* md3dDevice;						// D3D11 ����̽�
    ID3D11DeviceContext* md3dImmediateContext;		// ����̽� ���ؽ�Ʈ
    IDXGISwapChain* mSwapChain;						// ����ü��
    ID3D11Texture2D* mDepthStencilBuffer;			// ���� ���Ľ� ����
    // �� ���۸� ����� ���� ���� Ÿ�� ��,
    ID3D11RenderTargetView* mRenderTargetView;		// ���� Ÿ�� ��
    ID3D11DepthStencilView* mDepthStencilView;		// ���� ���Ľ� ��
    // ����� ���� �������� �����ϴ� ���� �� ��Ʈ��. ��Ʈ�� �� �״�� �ױ��� �����ϴ� ���� ���ڴ�.
    D3D11_VIEWPORT mScreenViewport;					// ����Ʈ


    /// ���� WinMain���� ���� ���� ������ ���� ���� ���� ������.
    // Derived class should set these in derived constructor to customize starting values.
    std::wstring mMainWndCaption;		/// �̷����̳�
    D3D_DRIVER_TYPE md3dDriverType;

public:
    int mClientWidth;					///
    int mClientHeight;					/// �̷� �͵�

private:
    bool mEnable4xMsaa;


    // �ʱ�ȭ ���� ����ü��
    D3D_FEATURE_LEVEL featureLevel;


    // DXTKFont
private:
    DXTKFont* m_pFont;

    // Camera
private:
    Camera* m_pCamera;

    // 	float mTheta;
    // 	float mPhi;
    // 	float mRadius;

    POINT mLastMousePos;

    virtual void OnMouseDown(int x, int y);
    virtual void OnMouseUp(int x, int y);
    virtual void OnMouseMove(int btnState, int x, int y);

    // Mesh (Box)
private:
    ID3D11Buffer* mBoxVB;
    ID3D11Buffer* mBoxIB;

    ID3DX11Effect* mFX;
    ID3DX11EffectTechnique* mTech;
    ID3DX11EffectMatrixVariable* mfxWorldViewProj;

    ID3D11InputLayout* mInputLayout;

    XMFLOAT4X4 mWorld;	// Transform Matrix
    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;

private:
    //void BuildGeometryBuffers();
    void BuildFX();
    void BuildFX_Create();
    void BuildVertexLayout();


    /// Render State
    // �̸� ���� ��Ʈ�� �����ΰ� ����Ī�Ѵ�.
private:
    ID3D11RasterizerState* mWireframeRS;
    ID3D11RasterizerState* mSolidRS;

    // ��Ʈ������ �������Ľ� ������Ʈ�� ������ �ƴ�.
    ID3D11DepthStencilState* NormalDSS;


private:
    void CreateRenderStates();


    /// Axis�� Grid�� helper�׸���
private:
    Axis* m_WorldAxes;
    Grid* m_Grid;


    /// ����� ����
private:
    DXGI_ADAPTER_DESC1 m_AdapterDesc1;
    HRESULT GetAdapterInfo();


    /// �ذ� �׸���. �޽� ������Ʈ. Object�� ���� ���� Render�ϴ� ����
private:
    MeshObject* m_MeshObject;


    /// ���ÿ� �ڽ�
private:
    Box* m_TestBox;

    Crate* m_TestCrate;
    //SiegeTank* m_SiegeTank;
    DebugSphere* m_debugObj_Point;
    DebugCone* m_debugObj_Spot;
    DebugArrow* m_debugObj_Spot2;
    DebugArrow* m_debugObj_Dir;

    ASEParser* m_pASEParser;
private:
    // changes as assignment changes
    void Update_LightsControl(float delta);
    bool m_isDirectionalLightActive = false;
    bool m_isPointLightActive = false;
    bool m_isSpotLightActive = false;
    bool m_isWhiteLight = false;
    DirectionalLight mDirLights[3];
    PointLight mPointLights[3];
    SpotLight mSpotLights[3];
    template<int key>
    bool WasKeyJustPressed()
    {
        static bool keyPressed = false;
        if (GetAsyncKeyState(key) & 0x8000)
        {
            if (keyPressed)
                return false;
            keyPressed = true;
            return true;
        }
        else
        {
            keyPressed = false;
            return false;
        }
    }
};


