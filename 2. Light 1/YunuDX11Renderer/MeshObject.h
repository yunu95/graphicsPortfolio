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
/// 메쉬
/// 쉐이더 정리가 되기 전까지는 계속 스펙(IA, Shader)이 업그레이드 될 것이다.
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
        void BuildGeometryBuffers2();		// 기하구조로부터 버텍스/인덱스버퍼를 만든다.


    private:
        ID3D11Device* md3dDevice;						// D3D11 디바이스
        ID3D11DeviceContext* md3dImmediateContext;		// 디바이스 컨텍스트

        ID3D11Buffer* mVB;
        ID3D11Buffer* mIB;

        // Transform Matrix
        XMFLOAT4X4 mWorld;	// Define transformations from local spaces to world space.
        XMFLOAT4X4 mView;
        XMFLOAT4X4 mProj;

        // 렌더스테이트도 바깥에서 받아온다.
        ID3D11RasterizerState* m_pRenderstate;

        UINT IndexCount;
        Material m_Material;

        /// 빛은 외부에서 받아오도록 하자
        DirectionalLight mDirLights[3];
        PointLight mPointLights[3];
        SpotLight mSpotLights[3];
        XMFLOAT3 mEyePosW;
        //UINT mLightCount = 3;


        ///----------------------------------------------------------------------------------------------------
        /// 텍스쳐에서 추가된 부분
        ID3D11ShaderResourceView* mDiffuseMapSRV;

        XMFLOAT4X4 mTexTransform;
        XMFLOAT4X4 mBoxWorld;

        int mBoxVertexOffset;
        UINT mBoxIndexOffset;
        UINT mBoxIndexCount;
        UINT mvcount = 0;
    };
}
