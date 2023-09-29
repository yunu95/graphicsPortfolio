#pragma once

#include <map>
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
        void LoadGeometry(ASE::Mesh* meshData, const XMMATRIX& basisTransform = XMMatrixIdentity());
        void LoadGeometry(vector<ASE::Mesh*> meshDatas);
        void SetRootTM(const XMMATRIX& tm);
        XMMATRIX GetRootTM();
        XMMATRIX GetBoneWorldTM(const string& boneName)const;
        void SetBoneLocalTM(const string& boneName, const XMMATRIX& tm);
        XMMATRIX GetBoneLocalTM(const string& boneName)const;
        void Update(Camera* pCamera);
        void Render();
        ID3DX11EffectTechnique* mTech;


    public:
        void BuildGeometryBuffers2();		// ���ϱ����κ��� ���ؽ�/�ε������۸� �����.


    private:
        XMMATRIX GetBoneWorldTM(int boneIndex)const;
        XMMATRIX m_rootTM = XMMatrixIdentity();
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

        // Mth bone is the parent bone of Nth node when parentIndexes[n] == M is true.
        vector<int> m_nodeParents;
        vector<XMMATRIX> m_nodeLocalTM;
        mutable vector<XMMATRIX> m_nodeWorldTM;
        mutable vector<bool> m_nodeWorldTMCached;
        vector<int> m_nodeVertexSize;
        vector<int> m_nodeVertexOffset;
        map<std::string, int> m_nodeIndexByName;
        int m_nodeSize;

        int mBoxVertexOffset;
        UINT mBoxIndexOffset;
        UINT mBoxIndexCount;
        UINT mvcount = 0;
    };
}
