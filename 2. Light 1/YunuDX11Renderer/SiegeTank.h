#pragma once

#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech
#include "Vertex.h"
#include "InputLayout.h"
#include "Effects.h"

class Camera;

/// <summary>
/// ��å ������ ��� ������Ʈ�� ����°��� �����ֱ� ���� ����
/// 7���� SiegeTank������ �����Ѵ�.
///
/// �������� �ּ��� �� �޾ƺô�.
/// 
/// 2021.04.05 LeHideKGIT
/// </summary>
namespace render_object
{
    class SiegeTank
    {
    public:
        SiegeTank(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS);
        ~SiegeTank();

        struct Vertex
        {
            XMFLOAT3 Pos;
            XMFLOAT4 Color;
        };

    public:
        void Initialize();	// �ʱ�ȭ
        void Update(Camera* pCamera);		// ����� ������Ʈ �� ��ȯ��ĸ� �����Ѵ�.
        void Render();		// �׸���. ������ BeginRender()�� EndRender()���̿� ȣ�� �� ���� ����Ѵ�.


    private:
        void BuildGeometryBuffers();		// ���ϱ����κ��� ���ؽ�/�ε������۸� �����.


        /// �ڽ��� �׸��� ���ؼ��� ���� �־�� �ϳ�..
        // �Ϻδ� �����͸� (�����ּ�)
        // �Ϻδ� ���Ⱑ ���� (������ �ڿ� ������ ���� ���Ⱑ �ƴ� ���� �ִ�)
    private:
        ID3D11Device* md3dDevice;						/// D3D11 ����̽�
        ID3D11DeviceContext* md3dImmediateContext;		/// ����̽� ���ؽ�Ʈ

        ID3D11Buffer* mVB;	// ���ؽ�����
        ID3D11Buffer* mIB;	// �ε�������

        ID3DX11Effect* mFX;								// ����Ʈ
        ID3DX11EffectTechnique* mTech;					// ��ũ

        ID3D11InputLayout* mInputLayout;				// ��ǲ���̾ƿ�

        // ��ȯ ����
        XMFLOAT4X4 mWorld;	// ���� ��ȯ ��� (����->����)
        XMFLOAT4X4 mView;	// �þ� ��ȯ ��� (ī�޶� ��)
        XMFLOAT4X4 mProj;	// ���� ��ȯ ��� (����/����)


        // ����������Ʈ. �������� ��� �� ���ΰ��� ���� ��.
        ID3D11RasterizerState* m_pRenderstate;			/// �ܺο��� �����ؼ� ������ ��Ȳ�� ���� ������. ���̴����� �ص� �ȴ�.


        ///----------------------------------------------------------------------------------------------------
        /// Light���� �߰��� �κ�
        DirectionalLight mDirLights[3];
        Material mBoxMat;
        XMFLOAT3 mEyePosW;

        ///----------------------------------------------------------------------------------------------------
        /// �ؽ��Ŀ��� �߰��� �κ�
        ID3D11ShaderResourceView* mDiffuseMapSRV;

        XMFLOAT4X4 mTexTransform;
        XMFLOAT4X4 mBoxWorld;

        int mBoxVertexOffset;
        UINT mBoxIndexOffset;
        UINT mBoxIndexCount;

    };
}
