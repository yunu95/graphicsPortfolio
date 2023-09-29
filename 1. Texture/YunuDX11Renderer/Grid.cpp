#include "Grid.h"
#include <DirectXColors.h>	// �̹� ���� �κ� DX11�� ��ģ��.

#ifdef _DEBUG
#pragma comment( lib, "Lib/Effects11d.lib" )
#else
#pragma comment( lib, "Lib/Effects11.lib" )
#endif

Grid::Grid(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS)
	: md3dDevice(pDevice), md3dImmediateContext(pDeviceContext), m_pRenderstate(pRS),
	mVB(nullptr), mIB(nullptr), mFX(nullptr), mTech(nullptr), mfxWorldViewProj(nullptr), mInputLayout(nullptr),
	mWorld(), mView(), mProj()
{

}

Grid::~Grid()
{
	mFX->Release();
	mInputLayout->Release();

	mVB->Release();
	mIB->Release();
}


void Grid::Initialize()
{
	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
}

void Grid::Update(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj)
{
	mWorld = world;
	mView = view;
	mProj = proj;
}

void Grid::Render()
{
	// �Է� ��ġ ��ü ����
	md3dImmediateContext->IASetInputLayout(mInputLayout);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	// �ε������ۿ� ���ؽ����� ����
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);

	/// WVP TM���� ����
	// Set constants
	XMMATRIX worldViewProj = mWorld * mView * mProj;
	mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));

	// ����������Ʈ
	md3dImmediateContext->RSSetState(m_pRenderstate);

	// ��ũ����...
	D3DX11_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);

	// �����н���...
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// 20���� �ε����� �׸��带 �׸���.
		md3dImmediateContext->DrawIndexed(40, 0, 0);
	}
}

void Grid::BuildGeometryBuffers()
{
	// ���� ���۸� �����Ѵ�. 
	// 40���� ������ �������.
	Vertex vertices[100];
	for (int i = 0; i < 100; i++)
	{
		vertices[i].Pos = XMFLOAT3((float)(i % 10) - 5.0f, 0.0f, (float)(i / 10) - 5.0f);
		vertices[i].Color = XMFLOAT4((const float*)&Colors::DarkGray);	// (��ο� ȸ��)
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex) * 100;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));


	// �ε��� ���۸� �����Ѵ�.
	// ���� 40���� ������ ��Ÿ������ �ߴ�.
	UINT indices[40];
 	for (int i = 0; i < 10; i++)
 	{
 		indices[i * 2] = i;
 		indices[i * 2 + 1] = i + 90;
 	}

	for (int i = 0; i < 10; i++)
	{
		indices[20 + (i * 2)] = i * 10;
		indices[20 + (i * 2) + 1] = i * 10 + 9;
	}

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * 40;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));
}

void Grid::BuildFX()
{
	std::ifstream fin("fx/color.fxo", std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, md3dDevice, &mFX));

	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

void Grid::BuildVertexLayout()
{
	// Create the vertex input layout.
	D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	// Create the input layout
	D3DX11_PASS_DESC passDesc;
	mTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &mInputLayout));
}
