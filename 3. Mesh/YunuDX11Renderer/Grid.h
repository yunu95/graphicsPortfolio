#pragma once

#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech

/// <summary>
/// 3D공간의 그리드를 표현
/// 공간상의 위치를 잡는데 유용하다.
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


	// 뭐뭐 있어야 하니..
private:
	ID3D11Device* md3dDevice;						// D3D11 디바이스
	ID3D11DeviceContext* md3dImmediateContext;		// 디바이스 컨텍스트

	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3DX11Effect* mFX;
	ID3DX11EffectTechnique* mTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;

	ID3D11InputLayout* mInputLayout;

	XMMATRIX mWorld;	// Transform Matrix
	XMMATRIX mView;
	XMMATRIX mProj;

	// 와이어로 그리기 위한 RenderState. 쉐이더에서 해도 된다.
	// 예제에서 On/Off로 바꾸는 등의 일을 하는 경우 이것을 바꿔줘도 될 것 같다.
	// 쉐이더에서 하는 경우도 스위칭 비용도 줄일 수 있는 방법은 많다.
	ID3D11RasterizerState* m_pRenderstate;;


};

