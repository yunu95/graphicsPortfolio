#include "Crate.h"

#include <DirectXColors.h>
#include "DDSTextureLoader.h"
// Geometry는 물체의 양적 형태를 생성하기 위해 쓰는 네임스페이스다.
#include "Geometry.h"
// 카메라
#include "Camera.h"

Crate::Crate(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS)
    : md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
    mVB(nullptr), mIB(nullptr), mFX(nullptr), mTech(nullptr), mInputLayout(nullptr),
    mWorld(), mView(), mProj(),
    mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mBoxWorld, I);
    XMStoreFloat4x4(&mTexTransform, I);

    //mDirLights[0].Ambient = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
    //mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    //mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
    //mDirLights[0].Direction = XMFLOAT3(-1.f, 0.f, 0.0f);

    mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    mDirLights[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    mDirLights[1].Direction = XMFLOAT3(0.f, -1.f, 0.0f);

    mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    mDirLights[2].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    mDirLights[2].Direction = XMFLOAT3(0.f, 1.f, 0.0f);

    mPointLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mPointLights[0].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
    mPointLights[0].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
    mPointLights[0].Att = XMFLOAT3(0, 0, 1);
    mPointLights[0].Range = 10;

    mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(5.0f, 5.0f, 5.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

Crate::~Crate()
{
    /// 릴리즈/삭제는 할당/생성의 반대 순서로
    if (mFX)
        mFX->Release();
    if (mInputLayout)
        mInputLayout->Release();

    mIB->Release();
    mVB->Release();

    mDiffuseMapSRV->Release();

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

void Crate::Initialize()
{
    // Must init Effects first since InputLayouts depend on shader signatures.
    Effects::InitAll(md3dDevice);
    InputLayouts::InitAll(md3dDevice);

    ID3D11Resource* texResource = nullptr;
    HR(CreateDDSTextureFromFile(md3dDevice,
        L"Textures/WoodCrate01.dds", &texResource, &mDiffuseMapSRV));
    texResource->Release();	// view saves reference

    BuildGeometryBuffers();
}

void Crate::Update(Camera* pCamera, float deltaTime)
{
    XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
    XMStoreFloat4x4(&mView, pCamera->View());
    XMStoreFloat4x4(&mProj, pCamera->Proj());
    mLightRadian += deltaTime * 0.25f * YunuMath::Pi;
    XMVECTOR direction = XMVectorSet(cos(mLightRadian), -1, sin(mLightRadian), 0);
    direction = XMVector3Normalize(direction);
    mDirLights[0].Direction.x = XMVectorGetX(direction);
    mDirLights[0].Direction.y = XMVectorGetY(direction);
    mDirLights[0].Direction.z = XMVectorGetZ(direction);

    mPointLights[0].Position.x = 2 * XMVectorGetX(direction);
    mPointLights[0].Position.y = 2 + XMVectorGetX(direction);
    mPointLights[0].Position.z = 2 * XMVectorGetZ(direction);

    mEyePosW = XMFLOAT3(pCamera->GetPosition().x, pCamera->GetPosition().y, pCamera->GetPosition().z);
}

void Crate::Render()
{
    // 입력 배치 객체 셋팅
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 렌더 스테이트
    md3dImmediateContext->RSSetState(m_pRenderstate);

    // 버텍스버퍼와 인덱스버퍼 셋팅
    UINT stride = sizeof(VertexStruct::Basic32);
    UINT offset = 0;

    /// WVP TM등을 셋팅
    // Set constants
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX worldViewProj = world * view * proj;

    // Set per frame constants.
    Effects::BasicTexFX->SetDirLights(mDirLights);
    Effects::BasicTexFX->SetPointLights(mPointLights);
    Effects::BasicTexFX->SetSpotLights(mSpotLights);
    Effects::BasicTexFX->SetEyePosW(mEyePosW);

    ID3DX11EffectTechnique* activeTech = Effects::BasicTexFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

        // Draw the box.
        XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
        XMMATRIX worldInvTranspose = YunuMath::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::BasicTexFX->SetWorld(world);
        Effects::BasicTexFX->SetWorldInvTranspose(worldInvTranspose);
        Effects::BasicTexFX->SetWorldViewProj(worldViewProj);
        Effects::BasicTexFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
        Effects::BasicTexFX->SetMaterial(mBoxMat);
        Effects::BasicTexFX->SetDiffuseMap(mDiffuseMapSRV);

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
    }
}

void Crate::BuildGeometryBuffers()
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

    UINT totalIndexCount = mBoxIndexCount;

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
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

