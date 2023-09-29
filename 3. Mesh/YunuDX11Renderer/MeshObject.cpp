#include "MeshObject.h"

#include "Camera.h"
#include "DDSTextureLoader.h"
#include "Geometry.h"


MeshObject::MeshObject(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS)
    : md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
    mVB(nullptr), mIB(nullptr),
    mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f)
{
    XMStoreFloat4x4(&mTexTransform, XMMatrixIdentity());
}

MeshObject::~MeshObject()
{
    ReleaseCOM(mDiffuseMapSRV);

    ReleaseCOM(mIB);
    ReleaseCOM(mVB);
}

void MeshObject::Initialize()
{
    mDirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mDirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    mDirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    mDirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);

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
    L"Textures/000000002405_reverse.dds", &texResource, & mDiffuseMapSRV));
    texResource->Release();	// view saves reference

    //L"../Textures/WoodCrate01.dds", & texResource, & mDiffuseMapSRV));

    //ReleaseCOM(texResource);	// view saves reference
}

// ASE 파서에서 버텍스 정보, 인덱스 정보를 얻어온다.
void MeshObject::LoadGeomerty(ASE::Mesh* meshData)
{
    UINT tcount = 0;
    UINT faceCount = 0;

    mvcount = meshData->m_meshface.size() * 3;
    faceCount = meshData->m_meshface.size();
    std::vector<VertexStruct::Basic32> vertices(mvcount);

    //for (ASE::Vertex* vertex : meshData->m_meshvertex)
    for (int i = 0; i < faceCount; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            auto posIndex = meshData->m_meshface[i]->m_vertexindex[j];
            auto texIndex = meshData->m_meshface[i]->m_TFace[j];
            vertices[i * 3 + j].Pos = meshData->m_meshvertex[posIndex]->m_pos;
            vertices[i * 3 + j].Normal = meshData->m_meshface[i]->m_normalvertex[j];
            if (!meshData->m_mesh_tvertex.empty())
            {
                vertices[i * 3 + j].Tex.x = meshData->m_mesh_tvertex[texIndex]->m_u;
                vertices[i * 3 + j].Tex.y = meshData->m_mesh_tvertex[texIndex]->m_v;
            }
        }

        /*vertices[i].Normal.x = meshData->m_meshvertex[i]->m_normal.x;
        vertices[i].Normal.y = meshData->m_meshvertex[i]->m_normal.y;
        vertices[i].Normal.z = meshData->m_meshvertex[i]->m_normal.z;*/

        //vertices[i].Tex.x = meshData->m_meshvertex[i]->u;
        //vertices[i].Tex.y = meshData->m_meshvertex[i]->v;
    }

    tcount = meshData->m_mesh_numfaces;

    IndexCount = 3 * tcount;
    std::vector<UINT> indices(IndexCount);
    for (UINT i = 0; i < tcount; ++i)
    {
        indices[i * 3 + 0] = i * 3 + 0;
        indices[i * 3 + 1] = i * 3 + 1;
        indices[i * 3 + 2] = i * 3 + 2;

        //indices[i * 3 + 0] = meshData->m_opt_index[i].index[0];
        //indices[i * 3 + 1] = meshData->m_opt_index[i].index[2];
        //indices[i * 3 + 2] = meshData->m_opt_index[i].index[1];

        // 최적화라는게 당최 뭔지 모르겠으니, 일단 원문을 주석처리한다.
        //indices[i * 3 + 0] = meshData->m_opt_index[i].index[0];
        //indices[i * 3 + 1] = meshData->m_opt_index[i].index[2];
        //indices[i * 3 + 2] = meshData->m_opt_index[i].index[1];
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

void MeshObject::Update(Camera* pCamera)
{
    XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
    XMStoreFloat4x4(&mView, pCamera->View());
    XMStoreFloat4x4(&mProj, pCamera->Proj());

    //mWorld._11 = 10.0f;
    //mWorld._22 = 10.0f;
    //mWorld._33 = 10.0f;

    //mWorld._43 = 10.0f;

    //
    // Switch the number of lights based on key presses.
    //
    if (GetAsyncKeyState('0') & 0x8000)
        mLightCount = 0;

    if (GetAsyncKeyState('1') & 0x8000)
        mLightCount = 1;

    if (GetAsyncKeyState('2') & 0x8000)
        mLightCount = 2;

    if (GetAsyncKeyState('3') & 0x8000)
        mLightCount = 3;

    if (GetAsyncKeyState('4') & 0x8000)
        mLightCount = 4;

    if (GetAsyncKeyState('5') & 0x8000)
        mLightCount = 5;

    if (GetAsyncKeyState('6') & 0x8000)
        mLightCount = 6;

    if (GetAsyncKeyState('7') & 0x8000)
        mLightCount = 7;

    mEyePosW = XMFLOAT3(pCamera->GetPosition().x, pCamera->GetPosition().y, pCamera->GetPosition().z);
}

void MeshObject::Render()
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

    // 월드의 역행렬
    XMMATRIX worldInvTranspose = YunuMath::InverseTranspose(world);

    // Set per frame constants.
    Effects::BasicTexFX->SetDirLights(mDirLights);
    Effects::BasicTexFX->SetEyePosW(mEyePosW);

    // Figure out which technique to use.
    ID3DX11EffectTechnique* mTech = Effects::BasicTexFX->Light1Tech;
    switch (mLightCount)
    {
    case 1:
        mTech = Effects::BasicTexFX->Light1Tech;
        break;
    case 2:
        mTech = Effects::BasicTexFX->Light2Tech;
        break;
    case 3:
        mTech = Effects::BasicTexFX->Light3Tech;
        break;
    case 4:
        mTech = Effects::BasicTexFX->Light0TexTech;
        break;
    case 5:
        mTech = Effects::BasicTexFX->Light1TexTech;
        break;
    case 6:
        mTech = Effects::BasicTexFX->Light2TexTech;
        break;
    case 7:
        mTech = Effects::BasicTexFX->Light3TexTech;
        break;
    }

    // 테크닉은...
    D3DX11_TECHNIQUE_DESC techDesc;
    mTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
        // won't be using index buffer
        //md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

        // WVP는 위에서
        worldInvTranspose = YunuMath::InverseTranspose(world);

        Effects::BasicTexFX->SetWorld(world);
        Effects::BasicTexFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicTexFX->SetWorldViewProj(worldViewProj);
        Effects::BasicTexFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
        Effects::BasicTexFX->SetMaterial(m_Material);
        Effects::BasicTexFX->SetDiffuseMap(mDiffuseMapSRV);

        mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        //md3dImmediateContext->Draw(mvcount, 0);
        md3dImmediateContext->DrawIndexed(IndexCount, 0, 0);
    }
}

void MeshObject::BuildGeometryBuffers2()
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
