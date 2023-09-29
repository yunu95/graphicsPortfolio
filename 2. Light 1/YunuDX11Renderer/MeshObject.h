#pragma once

#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech
#include "Vertex.h"
#include "InputLayout.h"
#include "Effects.h"
#include "../Yunu3DLib/ParsingDataClass.h"


class Camera;
//struct ASEParser::Mesh;

/// <summary>
/// �޽�
/// ���̴� ������ �Ǳ� �������� ��� ����(IA, Shader)�� ���׷��̵� �� ���̴�.
///
/// 2021.03.24 LeHideOffice
/// </summary>
namespace render_object
{
    class MeshObject
    {
    public:
        MeshObject(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS);
        ~MeshObject();

        struct Vertex
        {
            XMFLOAT3 Pos;
            XMFLOAT4 Color;
        };

    public:
        void Initialize();
        void LoadGeomerty(ASE::Mesh* meshData);
        void Update(Camera* pCamera);
        void Render();


    public:
        void BuildGeometryBuffers2();		// ���ϱ����κ��� ���ؽ�/�ε������۸� �����.


    private:
        ID3D11Device* md3dDevice;						// D3D11 ����̽�
        ID3D11DeviceContext* md3dImmediateContext;		// ����̽� ���ؽ�Ʈ

        ID3D11Buffer* mVB;
        ID3D11Buffer* mIB;

        // Transform Matrix
        XMFLOAT4X4 mWorld;	// Define transformations from local spaces to world space.
        XMFLOAT4X4 mView;
        XMFLOAT4X4 mProj;

        // ����������Ʈ�� �ٱ����� �޾ƿ´�.
        ID3D11RasterizerState* m_pRenderstate;

        UINT IndexCount;
        Material m_Material;

        /// ���� �ܺο��� �޾ƿ����� ����
        DirectionalLight mDirLights[3];
        PointLight mPointLights[3];
        SpotLight mSpotLights[3];
        XMFLOAT3 mEyePosW;
        //UINT mLightCount = 3;


        ///----------------------------------------------------------------------------------------------------
        /// �ؽ��Ŀ��� �߰��� �κ�
        ID3D11ShaderResourceView* mDiffuseMapSRV;

        XMFLOAT4X4 mTexTransform;
        XMFLOAT4X4 mBoxWorld;

        int mBoxVertexOffset;
        UINT mBoxIndexOffset;
        UINT mBoxIndexCount;
        UINT mvcount = 0;
    };
}
