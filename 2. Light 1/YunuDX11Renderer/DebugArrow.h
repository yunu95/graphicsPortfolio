#pragma once

#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech
#include "Vertex.h"
#include "InputLayout.h"
#include "Effects.h"

class Camera;

namespace render_object
{
    namespace Helper
    {
        class DebugArrow
        {
        public:
            DebugArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS,
                XMFLOAT4 Color, float cylinderRadius = 0.1f, const XMFLOAT3& displacement = { 0.f,1.f,0.f }, const XMFLOAT3& position = { 0.f,0.f,0.f });
            ~DebugArrow();

            struct Vertex
            {
                XMFLOAT3 Pos;
                XMFLOAT4 Color;
            };

        public:
            void SetArrow(const XMVECTOR& displacement, const XMVECTOR& position);
            void SetArrow(const XMFLOAT3& displacement, const XMFLOAT3& position);
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

            ID3D11Buffer* mVB_Cylinder;	// ���ؽ�����
            ID3D11Buffer* mIB_Cylinder;	// �ε�������

            ID3DX11Effect* mFX;								// ����Ʈ
            ID3DX11EffectTechnique* mTech;					// ��ũ

            ID3D11InputLayout* mInputLayout;				// ��ǲ���̾ƿ�

            // ��ȯ ����
            XMFLOAT4X4 mWorld;	// ���� ��ȯ ��� (����->����)
            XMFLOAT4X4 mView;	// �þ� ��ȯ ��� (ī�޶� ��)
            XMFLOAT4X4 mProj;	// ���� ��ȯ ��� (����/����)


            // ����������Ʈ. �������� ��� �� ���ΰ��� ���� ��.
            ID3D11RasterizerState* m_pRenderstate;			/// �ܺο��� �����ؼ� ������ ��Ȳ�� ���� ������. ���̴����� �ص� �ȴ�.

            XMFLOAT4X4 mCylinderLocalTM;
            XMFLOAT4X4 mConeLocalTM;

            int mCylinderVertexOffset = 0;
            UINT mCylinderIndexOffset = 0;
            UINT mCylinderIndexCount = 0;

            int mConeVertexOffset = 0;
            UINT mConeIndexOffset = 0;
            UINT mConeIndexCount = 0;

            XMFLOAT4 mColor;
            float m_cylinderRadius;
            float m_tipHeight;
        };
    }
}
