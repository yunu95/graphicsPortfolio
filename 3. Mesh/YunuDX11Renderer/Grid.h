#pragma once

#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech

/// <summary>
/// 3D������ �׸��带 ǥ��
/// �������� ��ġ�� ��µ� �����ϴ�.
/// 
/// 2021.03.05 LeHideOffice
/// </summary>
class Grid
{
public:
	Grid(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, ID3D11RasterizerState* pRS);
	~Grid();

	struct Vertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Color;
	};

public:
	void Initialize();
	void Update(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj);
	void Render();


private:
	void BuildGeometryBuffers();
	void BuildFX();
	void BuildVertexLayout();


	// ���� �־�� �ϴ�..
private:
	ID3D11Device* md3dDevice;						// D3D11 ����̽�
	ID3D11DeviceContext* md3dImmediateContext;		// ����̽� ���ؽ�Ʈ

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMMATRIX mWorld;	// Transform Matrix
	XMMATRIX mView;
	XMMATRIX mProj;

	// ���̾�� �׸��� ���� RenderState. ���̴����� �ص� �ȴ�.
	// �������� On/Off�� �ٲٴ� ���� ���� �ϴ� ��� �̰��� �ٲ��൵ �� �� ����.
	// ���̴����� �ϴ� ��쵵 ����Ī ��뵵 ���� �� �ִ� ����� ����.
	ID3D11RasterizerState* m_pRenderstate;;


};

