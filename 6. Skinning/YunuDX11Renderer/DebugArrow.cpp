#include <iostream>
#include <tuple>
#include "DebugArrow.h"

#include <DirectXColors.h>
#include "DDSTextureLoader.h"
// Geometry는 물체의 양적 형태를 생성하기 위해 쓰는 네임스페이스다.
#include "Geometry.h"
// 카메라
#include "Camera.h"
#include "PosEffect.h"

render_object::Helper::DebugArrow::DebugArrow(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext,
    ID3D11RasterizerState* pRS, XMFLOAT4 Color, float cylinderRadius, const XMFLOAT3& displacement, const XMFLOAT3& position)

    : md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
    mVB_Cylinder(nullptr), mIB_Cylinder(nullptr), mFX(nullptr), mTech(nullptr), mInputLayout(nullptr),
    mWorld(), mView(), mProj(), mColor(Color), m_cylinderRadius(cylinderRadius), m_tipHeight(cylinderRadius * 3)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mWorld, I);
    SetArrow(displacement, position);
}

render_object::Helper::DebugArrow::~DebugArrow()
{
    /// 릴리즈/삭제는 할당/생성의 반대 순서로
    if (mFX)
        mFX->Release();
    if (mInputLayout)
        mInputLayout->Release();

    mIB_Cylinder->Release();
    mVB_Cylinder->Release();

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}
void render_object::Helper::DebugArrow::SetArrow(const XMVECTOR& displacement, const XMVECTOR& position)
{
    XMFLOAT3 disp;
    XMFLOAT3 pos;
    XMStoreFloat3(&disp, displacement);
    XMStoreFloat3(&pos, position);
    SetArrow(disp, pos);
}
void render_object::Helper::DebugArrow::SetArrow(const XMFLOAT3& displacement, const XMFLOAT3& position)
{
    XMVECTOR displacementVector = XMLoadFloat3(&displacement);
    XMVECTOR positionVector = XMLoadFloat3(&position);
    float distance = XMVectorGetX(XMVector3LengthEst(displacementVector));
    XMStoreFloat4x4(&mCylinderLocalTM, XMMatrixScaling(1, distance - m_tipHeight, 1));
    XMStoreFloat4x4(&mConeLocalTM, XMMatrixTranslation(0, distance, 0));
    XMVECTOR rotationAxis = XMVector3Cross({ 0.f,1.f,0.f }, displacementVector);

    float angle = acos(XMVectorGetX(XMVector3Dot({ 0.f,1.f,0.f }, XMVector3NormalizeEst(displacementVector))));
    if (XMVector3Equal(rotationAxis, XMVectorZero()))
        rotationAxis = { 1,0,0,0 };
    XMStoreFloat4x4(&mWorld, XMMatrixRotationAxis(rotationAxis, angle) * XMMatrixTranslationFromVector(positionVector));
}

void render_object::Helper::DebugArrow::Initialize()
{
    // Must init Effects first since InputLayouts depend on shader signatures.
    //Effects::InitAll(md3dDevice);
    //InputLayouts::InitAll(md3dDevice);

    BuildGeometryBuffers();
}

void render_object::Helper::DebugArrow::Update(Camera* pCamera)
{
    XMStoreFloat4x4(&mView, pCamera->View());
    XMStoreFloat4x4(&mProj, pCamera->Proj());
}

void render_object::Helper::DebugArrow::Render()
{
    // 입력 배치 객체 셋팅
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 렌더 스테이트
    md3dImmediateContext->RSSetState(m_pRenderstate);

    // 버텍스버퍼와 인덱스버퍼 셋팅
    UINT stride = sizeof(VertexStruct::Pos);
    UINT offset = 0;

    /// WVP TM등을 셋팅
    // Set constants
    XMMATRIX view = XMLoadFloat4x4(&mView);
    XMMATRIX proj = XMLoadFloat4x4(&mProj);
    XMMATRIX world = XMLoadFloat4x4(&mWorld);
    XMMATRIX worldViewProj = world * view * proj;

    ID3DX11EffectTechnique* activeTech = Effects::Pos->colorTech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB_Cylinder, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mIB_Cylinder, DXGI_FORMAT_R32_UINT, 0);

        // Draw the box.
        XMMATRIX cylinderLocalTM = XMLoadFloat4x4(&mCylinderLocalTM);
        XMMATRIX coneLocalTM = XMLoadFloat4x4(&mConeLocalTM);
        XMMATRIX world = XMLoadFloat4x4(&mWorld);
        XMMATRIX worldInvTranspose = YunuMath::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::Pos->SetWorld(world);
        Effects::Pos->SetColor(mColor);
        Effects::Pos->SetWorldInvTranspose(worldInvTranspose);
        Effects::Pos->SetWorldViewProj(cylinderLocalTM * worldViewProj);

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mCylinderIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);

        Effects::Pos->SetWorldViewProj(coneLocalTM * worldViewProj);
        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mConeIndexCount, mConeIndexOffset, mConeVertexOffset);
        //md3dImmediateContext->DrawIndexed(mCylinderIndexCount+ mConeIndexCount, mCylinderIndexOffset, mCylinderVertexOffset);
    }
}

void render_object::Helper::DebugArrow::BuildGeometryBuffers()
{
    Geometry::MeshData mergedMesh;
    Geometry::MeshData cylinder;
    Geometry::MeshData cone;

    Geometry::CreateCylinder(m_cylinderRadius, m_cylinderRadius, 1, 15, 1, cylinder, XMFLOAT3(0.5f, 0.0f, 0.5f));
    Geometry::CreateCylinder(m_cylinderRadius * 3, 0, m_tipHeight, 15, 1, cone, XMFLOAT3(0.5f, 1.0f, 0.5f));
    mergedMesh = Geometry::MergeMesh(cylinder, cone);
    //mergedMesh = Geometry::MergeMesh(cone, cylinder);

    mCylinderVertexOffset = 0;
    mCylinderIndexCount = cylinder.Indices.size();
    mCylinderIndexOffset = 0;

    mConeVertexOffset = 0;
    mConeIndexCount = cone.Indices.size();
    mConeIndexOffset = mCylinderIndexCount;

    UINT totalVertexCount = mergedMesh.Vertices.size();

    UINT totalIndexCount = mergedMesh.Indices.size();

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<VertexStruct::Pos> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < mergedMesh.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = mergedMesh.Vertices[i].Position;
        //vertices[k].Color = { 1,0,0,1 };
        //vertices[k].Tex = sphere.Vertices[i].TexC;
        //vertices[k].Tex = sphere.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(VertexStruct::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB_Cylinder));

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    std::vector<UINT> indices;
    indices = std::move(mergedMesh.Indices);
    //indices.insert(indices.end(), mergedMesh.Indices.begin(), mergedMesh.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB_Cylinder));
}
