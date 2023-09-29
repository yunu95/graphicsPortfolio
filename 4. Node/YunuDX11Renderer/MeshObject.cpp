#include <unordered_map>
#include <stack>
#include "MeshObject.h"

#include "Camera.h"
#include "DDSTextureLoader.h"
#include "Geometry.h"


render_object::MeshObject::MeshObject(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS)
    : md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
    mVB(nullptr), mIB(nullptr),
    mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f)
{
    XMStoreFloat4x4(&mTexTransform, XMMatrixIdentity());
}

render_object::MeshObject::~MeshObject()
{
    ReleaseCOM(mDiffuseMapSRV);

    ReleaseCOM(mIB);
    ReleaseCOM(mVB);
}

void render_object::MeshObject::Initialize()
{
    //mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    //mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    //mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    //mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    //mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    //mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    //mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    //mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    //mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    //mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    //mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    //mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

    mSpotLights[0].Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 0.1f);
    mSpotLights[0].Diffuse = XMFLOAT4(100.0f, 100.0f, 100.f, 1.0f);
    mSpotLights[0].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mSpotLights[0].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
    mSpotLights[0].Att = XMFLOAT3(1, 0, 1);
    mSpotLights[0].Position = XMFLOAT3(0, 10, 10);
    mSpotLights[0].Range = 30;
    mSpotLights[0].Spot = 1;

    m_Material.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_Material.Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    m_Material.Specular = XMFLOAT4(0.8f, 0.8f, 0.8f, 16.0f);

    XMMATRIX skullScale = XMMatrixScaling(0.5f, 0.5f, 0.5f);
    XMMATRIX skullOffset = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&mWorld, XMMatrixMultiply(skullScale, skullOffset));

    ID3D11Resource* texResource = nullptr;
    //HR(CreateDDSTextureFromFile(md3dDevice,
        //L"Textures/WoodCrate01.dds", &texResource, &mDiffuseMapSRV));

    HR(CreateDDSTextureFromFile(md3dDevice,
        //L"Textures/WoodCrate01.dds", &texResource, &mDiffuseMapSRV));
    //L"Textures/000000002405.dds", &texResource, &mDiffuseMapSRV));
        L"Textures/000000002405_reverse.dds", &texResource, &mDiffuseMapSRV));
    texResource->Release();	// view saves reference

    //L"../Textures/WoodCrate01.dds", & texResource, & mDiffuseMapSRV));

    //ReleaseCOM(texResource);	// view saves reference
    //Effects::InitAll(md3dDevice);
    mTech = Effects::BasicTexFX->Light1TexTech;
    //mTech = Effects::BasicTexFX->DebugNormalTech;
}

// ASE 파서에서 버텍스 정보, 인덱스 정보를 얻어온다.
void render_object::MeshObject::LoadGeometry(ASE::Mesh* meshData, const XMMATRIX& basisTransform)
{
    LoadGeometry({ meshData }, basisTransform);
}
void render_object::MeshObject::LoadGeometry(vector<ASE::Mesh*> meshDatas)
{
    //UINT tcount = 0;
    m_nodeIndexByName.clear();
    mvcount = 0;
    int nodeIndex = 0;
    for (auto* meshData : meshDatas)
    {
        //mvcount += meshData->m_meshvertex.size();
        mvcount += meshData->m_mesh_numfaces * 3;
        m_nodeIndexByName[meshData->m_nodename] = nodeIndex;
        nodeIndex++;
    }
    m_nodeSize = nodeIndex;
    m_nodeParents.resize(m_nodeSize);
    m_nodeLocalTM.resize(m_nodeSize);
    m_nodeWorldTM.resize(m_nodeSize);
    m_nodeWorldTMCached.resize(m_nodeSize);
    m_nodeVertexSize.resize(m_nodeSize);
    m_nodeVertexOffset.resize(m_nodeSize);
    std::vector<VertexStruct::Basic32> vertices(mvcount);

    nodeIndex = 0;
    for (auto* meshData : meshDatas)
    {
        if (m_nodeIndexByName.find(meshData->m_nodeparent) != m_nodeIndexByName.end())
            m_nodeParents[nodeIndex] = m_nodeIndexByName[meshData->m_nodeparent];
        else
            m_nodeParents[nodeIndex] = -1;
        m_nodeWorldTM[nodeIndex] =
        {
            meshData->m_tm_row0.x,meshData->m_tm_row0.y,meshData->m_tm_row0.z, 0,
            meshData->m_tm_row1.x,meshData->m_tm_row1.y,meshData->m_tm_row1.z, 0,
            meshData->m_tm_row2.x,meshData->m_tm_row2.y,meshData->m_tm_row2.z, 0,
            meshData->m_tm_row3.x,meshData->m_tm_row3.y,meshData->m_tm_row3.z, 1
        };
        m_nodeVertexSize[nodeIndex] = meshData->m_mesh_numfaces * 3;
        m_nodeVertexOffset[nodeIndex] = 0;
        if (nodeIndex > 0)
        {
            m_nodeVertexOffset[nodeIndex] += m_nodeVertexOffset[nodeIndex - 1];
            m_nodeVertexOffset[nodeIndex] += m_nodeVertexSize[nodeIndex - 1];
        }
        nodeIndex++;
    }
    //XMMATRIX basisInvTrs = YunuMath::InverseTranspose(basisTransform);
    nodeIndex = 0;
    for (auto* meshData : meshDatas)
    {
        XMMATRIX WTMInvTrs = YunuMath::InverseTranspose(m_nodeWorldTM[nodeIndex]);
        XMMATRIX WTMInv = XMMatrixTranspose(WTMInvTrs);
        XMMATRIX parentWTMInv = XMMatrixIdentity();
        if (m_nodeParents[nodeIndex] >= 0)
            parentWTMInv = XMMatrixTranspose(YunuMath::InverseTranspose(m_nodeWorldTM[m_nodeParents[nodeIndex]]));
        m_nodeLocalTM[nodeIndex] = m_nodeWorldTM[nodeIndex] * parentWTMInv;
        //m_nodeWorldTM[nodeIndex] *= basisTransform;
        m_nodeWorldTMCached[nodeIndex] = true;
        UINT faceCount = 0;
        faceCount = meshData->m_meshface.size();

        for (int i = 0; i < faceCount; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                auto posIndex = meshData->m_meshface[i]->m_vertexindex[j];
                auto texIndex = meshData->m_meshface[i]->m_TFace[j];
                // apply inverse world transform right here!
                // especially, when it comes to normal, you must multiply inverse transpose world transform.
                XMVECTOR pos = meshData->m_meshvertex[posIndex]->m_pos;
                pos.vector4_f32[3] = 1;
                XMVECTOR normal = meshData->m_meshface[i]->m_normalvertex[j];

                pos = XMVector4Transform(pos, WTMInv);
                //normal = XMVector4Transform(normal, WTMInvTrs);
                //pos = XMVector3Transform(pos, basisTransform);
                //normal = XMVector3Transform(normal, basisInvTrs);

                XMStoreFloat3(&vertices[m_nodeVertexOffset[nodeIndex] + i * 3 + j].Pos, pos);
                XMStoreFloat3(&vertices[m_nodeVertexOffset[nodeIndex] + i * 3 + j].Normal, normal);
                if (!meshData->m_mesh_tvertex.empty())
                {
                    vertices[m_nodeVertexOffset[nodeIndex] + i * 3 + j].Tex.x = meshData->m_mesh_tvertex[texIndex]->m_u;
                    vertices[m_nodeVertexOffset[nodeIndex] + i * 3 + j].Tex.y = meshData->m_mesh_tvertex[texIndex]->m_v;
                }
            }
        }

        //UINT tcount = meshData->m_mesh_numfaces;

        //IndexCount = 3 * tcount;
        //std::vector<UINT> indices(IndexCount);
        /*for (UINT i = 0; i < tcount; ++i)
        {
            indices[i * 3 + 0] = i * 3 + 0;
            indices[i * 3 + 1] = i * 3 + 1;
            indices[i * 3 + 2] = i * 3 + 2;*/

            //indices[i * 3 + 0] = meshData->m_opt_index[i].index[0];
            //indices[i * 3 + 1] = meshData->m_opt_index[i].index[2];
            //indices[i * 3 + 2] = meshData->m_opt_index[i].index[1];

            // 최적화라는게 당최 뭔지 모르겠으니, 일단 원문을 주석처리한다.
            //indices[i * 3 + 0] = meshData->m_opt_index[i].index[0];
            //indices[i * 3 + 1] = meshData->m_opt_index[i].index[2];
            //indices[i * 3 + 2] = meshData->m_opt_index[i].index[1];
        //}
        nodeIndex++;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(VertexStruct::Basic32) * mvcount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

    //
    // Pack the indices of all the meshes into one index buffer.
    //
    // but it won't be necessary, cause we are gonna draw faces in order.

    //D3D11_BUFFER_DESC ibd;
    //ibd.Usage = D3D11_USAGE_IMMUTABLE;
    //ibd.ByteWidth = sizeof(UINT) * IndexCount;
    //ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    //ibd.CPUAccessFlags = 0;
    //ibd.MiscFlags = 0;
    //D3D11_SUBRESOURCE_DATA iinitData;
    //iinitData.pSysMem = &indices[0];
    //HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}
void render_object::MeshObject::SetRootTM(const XMMATRIX& tm)
{
    m_rootTM = tm;
}
XMMATRIX render_object::MeshObject::GetRootTM()
{
    return m_rootTM;
}
void render_object::MeshObject::SetBoneLocalTM(const string& boneName, const XMMATRIX& tm)
{
    if (m_nodeIndexByName.find(boneName) == m_nodeIndexByName.end())
        return;

    auto index = m_nodeIndexByName[boneName];
    m_nodeLocalTM[index] = tm;
}

XMMATRIX render_object::MeshObject::GetBoneWorldTM(const string& boneName)const
{
    return GetBoneWorldTM(m_nodeIndexByName.at(boneName)) * m_rootTM;
}
XMMATRIX render_object::MeshObject::GetBoneLocalTM(const string& boneName)const
{
    if (m_nodeIndexByName.find(boneName) == m_nodeIndexByName.end())
        return XMMatrixIdentity();

    auto index = m_nodeIndexByName.at(boneName);
    return m_nodeLocalTM[index];
}
void render_object::MeshObject::Update(Camera* pCamera)
{
    XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
    XMStoreFloat4x4(&mView, pCamera->View());
    XMStoreFloat4x4(&mProj, pCamera->Proj());
    for (auto each : m_nodeWorldTMCached)
        each = false;

    mEyePosW = XMFLOAT3(pCamera->GetPosition().x, pCamera->GetPosition().y, pCamera->GetPosition().z);
}

void render_object::MeshObject::Render()
{
    // 입력 배치 객체 셋팅
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 렌더 스테이트
    md3dImmediateContext->RSSetState(m_pRenderstate);

    // 버텍스버퍼와 인덱스버퍼 셋팅
    UINT stride = sizeof(VertexStruct::Basic32);
    UINT offset = 0;
    md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
    md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

    /// WVP TM등을 셋팅
    // Set constants
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX worldViewProj = world * view * proj;

    // 월드의 역전치행렬
    XMMATRIX worldInvTranspose = YunuMath::InverseTranspose(world);

    Effects::BasicTexFX->SetEyePosW(mEyePosW);

    // Figure out which technique to use.
    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);

        for (int i = 0; i < m_nodeSize; i++)
        {
            // won't be using index buffer
            //md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

            world = GetBoneWorldTM(i) * m_rootTM;
            worldViewProj = world * view * proj;
            worldInvTranspose = YunuMath::InverseTranspose(world);

            Effects::BasicTexFX->SetWorld(world);
            Effects::BasicTexFX->SetWorldInvTranspose(worldInvTranspose);
            Effects::BasicTexFX->SetWorldViewProj(worldViewProj);
            Effects::BasicTexFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
            Effects::BasicTexFX->SetMaterial(m_Material);
            Effects::BasicTexFX->SetDiffuseMap(mDiffuseMapSRV);

            mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
            md3dImmediateContext->Draw(m_nodeVertexSize[i], m_nodeVertexOffset[i]);
            //md3dImmediateContext->DrawIndexed(IndexCount, 0, 0);
        }
    }
}

void render_object::MeshObject::BuildGeometryBuffers2()
{
    Geometry::MeshData box;

    Geometry::CreateBox(1.0f, 1.0f, 1.0f, box);

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;

    // Cache the index count of each object.
    mBoxIndexCount = box.Indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;

    UINT totalVertexCount = box.Vertices.size();

    IndexCount = mBoxIndexCount;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<VertexStruct::Basic32> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = box.Vertices[i].Position;
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].Tex = box.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(VertexStruct::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * IndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}
XMMATRIX render_object::MeshObject::GetBoneWorldTM(int boneIndex)const
{
    if (m_nodeWorldTMCached[boneIndex])
        return m_nodeWorldTM[boneIndex];

    stack<int> hierarchy;
    hierarchy.push(boneIndex);
    XMMATRIX ret = XMMatrixIdentity();
    while (!m_nodeWorldTMCached[hierarchy.top()])
    {
        if (m_nodeParents[hierarchy.top()] < 0)
        {
            m_nodeWorldTM[hierarchy.top()] = m_nodeLocalTM[hierarchy.top()];
            //m_nodeWorldTMCached[hierarchy.top()] = true;
            break;
        }
        hierarchy.push(m_nodeParents[hierarchy.top()]);
    }
    ret = m_nodeWorldTM[hierarchy.top()];
    hierarchy.pop();

    while (!hierarchy.empty())
    {
        ret = m_nodeLocalTM[hierarchy.top()] * ret;
        m_nodeWorldTM[hierarchy.top()] = ret;
        //m_nodeWorldTMCached[hierarchy.top()] = true;
        hierarchy.pop();
    }
    return ret;
}
