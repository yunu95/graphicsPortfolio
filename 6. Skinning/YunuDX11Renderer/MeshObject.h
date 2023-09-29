#pragma once

#include <map>
#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech
#include "Vertex.h"
#include "InputLayout.h"
#include "Effects.h"
#include "../Yunu3DLib/Animation.h"
#include "../Yunu3DLib/ParsingDataClass.h"


class Camera;
//struct ASEParser::Mesh;

/// <summary>
// non-skinned mesh object.
// it saves its vertices position info relative to parent node.
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
        void LoadAnimation(const list<shared_ptr<Animation>>& nodeAnimList,const ASE::Scenedata& sceneData);
        void SetRootTM(const XMMATRIX& tm);
        XMMATRIX GetRootTM();
        XMMATRIX GetBoneWorldTM(const string& boneName)const;
        void SetBoneLocalTM(const string& boneName, const XMMATRIX& tm);
        XMMATRIX GetBoneLocalTM(const string& boneName)const;
        void Update(float deltaTime, Camera* pCamera);
        void Render();
        ID3DX11EffectTechnique* mTech;
        void SetAnimElapsed(float elapsed) { m_animElapsed = elapsed; }

    public:
        void BuildGeometryBuffers2();		// 기하구조로부터 버텍스/인덱스버퍼를 만든다.


    private:
        XMMATRIX GetBoneWorldTM(int boneIndex)const;
        void UpdateAnimation(float deltaTime);
        XMMATRIX m_rootTM = XMMatrixIdentity();
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

        // Mth bone is the parent bone of Nth node when parentIndexes[n] == M is true.
        vector<int> m_nodeParents;
        vector<XMMATRIX> m_nodeLocalTM;
        vector<XMVECTOR> m_nodeLocalInitialPos;
        vector<XMVECTOR> m_nodeLocalInitialRot;
        vector<XMVECTOR> m_nodeLocalInitialScl;

        // whole length of animation
        float m_animLength = 0;

        // duration of 1 frame of animation.
        float m_animTickLength = 0.0001f;

        // current elapsed time of animation.
        float m_animElapsed = 0;
        vector<shared_ptr<Animation>> m_Anim;
        vector<int> m_animPosIndices;
        vector<int> m_animRotIndices;
        vector<int> m_animScaleIndices;

        mutable vector<XMMATRIX> m_nodeWorldTM;
        mutable vector<bool> m_nodeWorldTMCached;
        vector<int> m_nodeVertexSize;
        vector<int> m_nodeVertexOffset;
        map<std::string, int> m_nodeIndexByName;
        int m_nodeSize;

        int mBoxVertexOffset;
        UINT mBoxIndexOffset;
        UINT mBoxIndexCount;
        UINT vertexCount = 0;
    };
}
