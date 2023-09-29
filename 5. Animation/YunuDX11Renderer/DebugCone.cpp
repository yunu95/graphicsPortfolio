#include "DebugCone.h"

#include <DirectXColors.h>
#include "DDSTextureLoader.h"
// Geometry�� ��ü�� ���� ���¸� �����ϱ� ���� ���� ���ӽ����̽���.
#include "Geometry.h"
// ī�޶�
#include "Camera.h"
#include "PosEffect.h"

render_object::Helper::DebugCone::DebugCone(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext,
    ID3D11RasterizerState* pRS, XMFLOAT4 Color, float radiusBottom, float height)
    : md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
    mVB(nullptr), mIB(nullptr), mFX(nullptr), mTech(nullptr), mInputLayout(nullptr),
    mWorld(), mView(), mProj(), mColor(Color), mRadiusBottom(radiusBottom), mHeight(height)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mWorld, I);
}

render_object::Helper::DebugCone::~DebugCone()
{
    /// ������/������ �Ҵ�/������ �ݴ� ������
    if (mFX)
        mFX->Release();
    if (mInputLayout)
        mInputLayout->Release();

    mIB->Release();
    mVB->Release();

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

void render_object::Helper::DebugCone::Initialize()
{
    // Must init Effects first since InputLayouts depend on shader signatures.
    //Effects::InitAll(md3dDevice);
    //InputLayouts::InitAll(md3dDevice);

    BuildGeometryBuffers();
}

void render_object::Helper::DebugCone::Update(Camera* pCamera)
{
    //XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
    XMStoreFloat4x4(&mView, pCamera->View());
    XMStoreFloat4x4(&mProj, pCamera->Proj());
}

void render_object::Helper::DebugCone::Render()
{
    // �Է� ��ġ ��ü ����
    md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
    md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // ���� ������Ʈ
    md3dImmediateContext->RSSetState(m_pRenderstate);

    // ���ؽ����ۿ� �ε������� ����
    UINT stride = sizeof(VertexStruct::Pos);
    UINT offset = 0;

    /// WVP TM���� ����
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
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

        // Draw the box.
        //XMMATRIX world = XMLoadFloat4x4(&mSphereWorld);
        XMMATRIX worldInvTranspose = YunuMath::InverseTranspose(world);
        XMMATRIX worldViewProj = world * view * proj;

        Effects::Pos->SetWorld(world);
        Effects::Pos->SetColor(mColor);
        Effects::Pos->SetWorldInvTranspose(worldInvTranspose);
        Effects::Pos->SetWorldViewProj(worldViewProj);

        activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
        md3dImmediateContext->DrawIndexed(mSphereIndexCount, mBoxIndexOffset, mBoxVertexOffset);
    }
}

void render_object::Helper::DebugCone::BuildGeometryBuffers()
{
    Geometry::MeshData cylinder;

    Geometry::CreateCylinder(mRadiusBottom, 0, mHeight, 20, 10, cylinder, XMFLOAT3(0.5f, 1.0f, 0.5f));

    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    mBoxVertexOffset = 0;

    // Cache the index count of each object.
    mSphereIndexCount = cylinder.Indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;

    UINT totalVertexCount = cylinder.Vertices.size();

    UINT totalIndexCount = mSphereIndexCount;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<VertexStruct::Pos> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = cylinder.Vertices[i].Position;
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
    HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));

    //
    // Pack the indices of all the meshes into one index buffer.
    //

    std::vector<UINT> indices;
    indices.insert(indices.end(), cylinder.Indices.begin(), cylinder.Indices.end());

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
