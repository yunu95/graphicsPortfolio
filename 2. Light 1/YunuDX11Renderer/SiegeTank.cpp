#include "SiegeTank.h"

#include <DirectXColors.h>
#include "DDSTextureLoader.h"
#include "Geometry.h"
#include "Camera.h"


render_object::SiegeTank::SiegeTank(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS)
    : md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
    mVB(nullptr), mIB(nullptr), mFX(nullptr), mTech(nullptr), mInputLayout(nullptr),
    mWorld(), mView(), mProj(),
    mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&mBoxWorld, I);
    XMStoreFloat4x4(&mTexTransform, I);

    mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
    mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
    mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

    mDirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
    mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
    mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

    mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

render_object::SiegeTank::~SiegeTank()
{
    /// 릴리즈/삭제는 할당/생성의 반대 순서로
    mFX->Release();
    mInputLayout->Release();

    mIB->Release();
    mVB->Release();

    mDiffuseMapSRV->Release();

    Effects::DestroyAll();
    InputLayouts::DestroyAll();
}

void render_object::SiegeTank::Initialize()
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

void render_object::SiegeTank::Update(Camera* pCamera)
{
    XMStoreFloat4x4(&mWorld, XMMatrixIdentity());
    XMStoreFloat4x4(&mView, pCamera->View());
    XMStoreFloat4x4(&mProj, pCamera->Proj());

    mEyePosW = XMFLOAT3(pCamera->GetPosition().x, pCamera->GetPosition().y, pCamera->GetPosition().z);
}

void render_object::SiegeTank::Render()
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
    Effects::BasicTexFX->SetEyePosW(mEyePosW);

    ID3DX11EffectTechnique* activeTech = Effects::BasicTexFX->Light2TexTech;

    D3DX11_TECHNIQUE_DESC techDesc;
    activeTech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
        md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

        // Draw the tank.
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

void render_object::SiegeTank::BuildGeometryBuffers()
{
    Geometry::MeshData tank;

    //Geometry::CreateBox(1.0f, 1.0f, 1.0f, tank);
    tank.Vertices = std::vector<Geometry::Vertex>(44, Geometry::Vertex());
    tank.Vertices[0].Position = XMFLOAT3(1.000000, 0.020514, -0.804146);
    tank.Vertices[1].Position = XMFLOAT3(0.265868, 0.294037, 0.804146);
    tank.Vertices[2].Position = XMFLOAT3(-1.000000, 0.020514, -0.804146);
    tank.Vertices[3].Position = XMFLOAT3(-0.206679, 0.294037, 0.804146);
    tank.Vertices[4].Position = XMFLOAT3(0.265868, 0.622103, 0.804146);
    tank.Vertices[5].Position = XMFLOAT3(-0.206679, 0.622103, 0.804146);
    tank.Vertices[6].Position = XMFLOAT3(0.782816, 0.631368, -0.586963);
    tank.Vertices[7].Position = XMFLOAT3(-0.782816, 0.631368, -0.586963);
    tank.Vertices[8].Position = XMFLOAT3(-1.000000, 0.020514, 0.804146);
    tank.Vertices[9].Position = XMFLOAT3(-0.782816, 0.631368, 0.586963);
    tank.Vertices[10].Position = XMFLOAT3(0.782816, 0.631368, 0.586963);
    tank.Vertices[11].Position = XMFLOAT3(1.000000, 0.020514, 0.804146);
    tank.Vertices[12].Position = XMFLOAT3(0.265868, 0.001457, 1.149454);
    tank.Vertices[13].Position = XMFLOAT3(-0.206679, 0.622103, 0.959410);
    tank.Vertices[14].Position = XMFLOAT3(0.265868, 0.622103, 0.959410);
    tank.Vertices[15].Position = XMFLOAT3(-0.206679, 0.001457, 1.149454);
    tank.Vertices[16].Position = XMFLOAT3(0.265868, 0.001042, 1.577618);
    tank.Vertices[17].Position = XMFLOAT3(-0.206679, 0.001042, 1.577618);
    tank.Vertices[18].Position = XMFLOAT3(1.000000, 0.631368, 0.804146);
    tank.Vertices[19].Position = XMFLOAT3(-1.000000, 0.631368, 0.804146);
    tank.Vertices[20].Position = XMFLOAT3(-1.000000, 0.631368, -0.804146);
    tank.Vertices[21].Position = XMFLOAT3(1.000000, 0.631368, -0.804146);
    tank.Vertices[22].Position = XMFLOAT3(0.582551, 0.988454, 0.304765);
    tank.Vertices[23].Position = XMFLOAT3(0.271304, 0.988454, 0.304765);
    tank.Vertices[24].Position = XMFLOAT3(0.271304, 0.988454, -0.304765);
    tank.Vertices[25].Position = XMFLOAT3(0.582551, 0.988454, -0.304765);
    tank.Vertices[26].Position = XMFLOAT3(0.782816, 0.988454, 0.586963);
    tank.Vertices[27].Position = XMFLOAT3(-0.782816, 0.988454, 0.586963);
    tank.Vertices[28].Position = XMFLOAT3(-0.782816, 0.988454, -0.586963);
    tank.Vertices[29].Position = XMFLOAT3(0.782816, 0.988454, -0.586963);
    tank.Vertices[30].Position = XMFLOAT3(0.271304, 1.208344, 0.304765);
    tank.Vertices[31].Position = XMFLOAT3(0.271304, 1.208344, -0.304765);
    tank.Vertices[32].Position = XMFLOAT3(1.151661, 1.908226, -0.238862);
    tank.Vertices[33].Position = XMFLOAT3(1.151661, 1.908226, 0.238862);
    tank.Vertices[34].Position = XMFLOAT3(0.948066, 2.052062, 0.238862);
    tank.Vertices[35].Position = XMFLOAT3(0.948066, 2.052062, -0.238862);
    tank.Vertices[36].Position = XMFLOAT3(1.205487, 1.870199, -0.304765);
    tank.Vertices[37].Position = XMFLOAT3(1.205487, 1.870199, 0.304765);
    tank.Vertices[38].Position = XMFLOAT3(0.894240, 2.090089, 0.304765);
    tank.Vertices[39].Position = XMFLOAT3(0.894240, 2.090089, -0.304765);
    tank.Vertices[40].Position = XMFLOAT3(0.639664, 1.098876, -0.238862);
    tank.Vertices[41].Position = XMFLOAT3(0.639664, 1.098876, 0.238862);
    tank.Vertices[42].Position = XMFLOAT3(0.436069, 1.242712, 0.238862);
    tank.Vertices[43].Position = XMFLOAT3(0.436069, 1.242712, -0.238862);


    tank.Vertices[0].TexC = XMFLOAT2(0.582062, 0.771028);
    tank.Vertices[1].TexC = XMFLOAT2(0.582062, 0.978972);
    tank.Vertices[2].TexC = XMFLOAT2(0.417938, 0.978972);
    tank.Vertices[3].TexC = XMFLOAT2(0.417938, 0.771028);
    tank.Vertices[4].TexC = XMFLOAT2(0.375000, 0.000000);
    tank.Vertices[5].TexC = XMFLOAT2(0.625000, 0.000000);
    tank.Vertices[6].TexC = XMFLOAT2(0.625000, 0.250000);
    tank.Vertices[7].TexC = XMFLOAT2(0.375000, 0.250000);
    tank.Vertices[8].TexC = XMFLOAT2(0.125000, 0.500000);
    tank.Vertices[9].TexC = XMFLOAT2(0.375000, 0.500000);
    tank.Vertices[10].TexC = XMFLOAT2(0.375000, 0.750000);
    tank.Vertices[11].TexC = XMFLOAT2(0.125000, 0.750000);
    tank.Vertices[12].TexC = XMFLOAT2(0.625000, 0.500000);
    tank.Vertices[13].TexC = XMFLOAT2(0.625000, 0.750000);
    tank.Vertices[14].TexC = XMFLOAT2(0.652148, 0.533760);
    tank.Vertices[15].TexC = XMFLOAT2(0.847852, 0.533760);
    tank.Vertices[16].TexC = XMFLOAT2(0.847852, 0.533760);
    tank.Vertices[17].TexC = XMFLOAT2(0.652148, 0.533760);
    tank.Vertices[18].TexC = XMFLOAT2(0.417938, 0.978972);
    tank.Vertices[19].TexC = XMFLOAT2(0.582062, 0.978972);
    tank.Vertices[20].TexC = XMFLOAT2(0.625000, 1.000000);
    tank.Vertices[21].TexC = XMFLOAT2(0.375000, 1.000000);
    tank.Vertices[22].TexC = XMFLOAT2(0.582062, 0.771028);
    tank.Vertices[23].TexC = XMFLOAT2(0.417938, 0.771028);
    tank.Vertices[24].TexC = XMFLOAT2(0.582062, 0.771028);
    tank.Vertices[25].TexC = XMFLOAT2(0.582062, 0.978972);
    tank.Vertices[26].TexC = XMFLOAT2(0.652148, 0.716240);
    tank.Vertices[27].TexC = XMFLOAT2(0.847852, 0.716240);
    tank.Vertices[28].TexC = XMFLOAT2(0.875000, 0.750000);
    tank.Vertices[29].TexC = XMFLOAT2(0.875000, 0.500000);
    tank.Vertices[30].TexC = XMFLOAT2(0.652148, 0.716240);
    tank.Vertices[31].TexC = XMFLOAT2(0.847852, 0.716240);
    tank.Vertices[32].TexC = XMFLOAT2(0.677181, 0.685110);
    tank.Vertices[33].TexC = XMFLOAT2(0.822819, 0.685110);
    tank.Vertices[34].TexC = XMFLOAT2(0.822819, 0.564890);
    tank.Vertices[35].TexC = XMFLOAT2(0.677181, 0.564890);
    tank.Vertices[36].TexC = XMFLOAT2(0.822819, 0.685110);
    tank.Vertices[37].TexC = XMFLOAT2(0.822819, 0.564890);
    tank.Vertices[38].TexC = XMFLOAT2(0.677181, 0.564890);
    tank.Vertices[39].TexC = XMFLOAT2(0.677181, 0.685110);
    tank.Vertices[40].TexC = XMFLOAT2(0.702367, 0.577888);
    tank.Vertices[41].TexC = XMFLOAT2(0.797633, 0.577888);
    tank.Vertices[42].TexC = XMFLOAT2(0.797633, 0.577888);
    tank.Vertices[43].TexC = XMFLOAT2(0.702367, 0.577888);

    tank.Indices = { 17,12,16,19,2,8,0,8,2,21,11,0,20,0,2,7,29,6,3,19,8,1,18,4,5,18,19,3,11,1,14,12,1,1,4,14,5,14,4,5,3,13,1,15,3,13,16,14,13,15,17,9,18,10,7,19,9,6,20,7,10,21,6,10,27,9,6,26,10,9,28,7,23,26,22,24,27,23,24,29,28,22,29,25,23,31,24,25,24,31,23,22,30,25,37,22,32,43,40,30,37,38,30,39,31,31,36,25,32,37,36,34,39,38,32,39,35,34,37,33,40,42,41,32,41,33,33,42,34,34,43,35,56,61,60,46,63,52,52,44,46,55,65,44,44,64,46,73,51,50,47,63,49,62,45,48,49,62,48,55,47,45,56,58,45,45,58,48,58,49,48,49,57,47,59,45,47,60,57,58,57,59,47,62,53,54,63,51,53,64,50,51,65,54,50,71,54,53,70,50,54,72,53,51,70,67,66,71,68,67,68,73,69,73,66,69,75,67,68,69,75,68,67,74,66,81,69,66,76,87,79,74,81,66,83,74,75,80,75,69,80,77,76,82,79,78,79,80,76,77,82,78,86,84,85,85,76,77,86,77,78,87,78,79,17,15,12,19,20,2,0,11,8,21,18,11,20,21,0,7,28,29,3,5,19,1,11,18,5,4,18,3,8,11,14,16,12,5,13,14,1,12,15,13,17,16,13,3,15,9,19,18,7,20,19,6,21,20,10,18,21,10,26,27,6,29,26,9,27,28,23,27,26,24,28,27,24,25,29,22,26,29,23,30,31,25,36,37,32,35,43,30,22,37,30,38,39,31,39,36,32,33,37,34,35,39,32,36,39,34,38,37,40,43,42,32,40,41,33,41,42,34,42,43,56,59,61,46,64,63,52,55,44,55,62,65,44,65,64,73,72,51,47,52,63,62,55,45,49,63,62,55,52,47,56,60,58,58,57,49,59,56,45,60,61,57,57,61,59,62,63,53,63,64,51,64,65,50,65,62,54,71,70,54,70,73,50,72,71,53,70,71,67,71,72,68,68,72,73,73,70,66,75,74,67,81,80,69,76,84,87,74,82,81,83,82,74,80,83,75,80,81,77,82,83,79,79,83,80,77,81,82,86,87,84,85,84,76,86,85,77,87,86,78 };
    // Cache the ertex offsets to each object in the concatenated vertex buffer.);
    mBoxVertexOffset = 0;

    // Cache the index count of each object.
    mBoxIndexCount = tank.Indices.size();

    // Cache the starting index for each object in the concatenated index buffer.
    mBoxIndexOffset = 0;

    UINT totalVertexCount = tank.Vertices.size();

    UINT totalIndexCount = mBoxIndexCount;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //

    std::vector<VertexStruct::Basic32> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < tank.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = tank.Vertices[i].Position;
        vertices[k].Normal = tank.Vertices[i].Normal;
        vertices[k].Tex = tank.Vertices[i].TexC;
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
    indices.insert(indices.end(), tank.Indices.begin(), tank.Indices.end());

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
