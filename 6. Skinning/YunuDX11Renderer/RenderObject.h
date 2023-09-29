#pragma once

#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech

// 모든 렌더 오브젝트들의 모체가 될 클래스
// 얘는 렌더 오브젝트 인스턴스별로 존재한다.
// 다만, 로컬 스페이스 버텍스 정보가 렌더 오브젝트마다 있어야 할까?
// 멤버 변수가 렌더 객체 타입마다 있어야 할지, 인스턴스마다 있어야 할 지 확신할 수 없기 때문에,
// 이 클래스는 부모가 될 자격이 없다.

// 개선안 : 로컬 스페이스의 버텍스 정보는 중첩되는 그룹들이 생기지 않게 정적 변수로 관리한다.

namespace render_object
{
	class RenderObject
	{
	public:
		RenderObject();
		~RenderObject();

		struct Vertex
		{
			XMFLOAT3 Pos;
			XMFLOAT4 Color;
		};

	public:
		void Initialize();
		void Update(const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& proj);
		void Render(ID3D11Device*& pDevice, ID3D11DeviceContext*& pDeviceContext, ID3D11RasterizerState*& pRS);


	private:
		void BuildGeometryBuffers();
		void BuildFX();
		void BuildVertexLayout();


		// 뭐뭐 있어야 하니..
	private:
		// 이 정보를 렌더 객체들이 각각 갖고 있어야 할까?
		//ID3D11Device* md3dDevice;						// D3D11 디바이스
		//ID3D11DeviceContext* md3dImmediateContext;		// 디바이스 컨텍스트

		// 타입이 같은 렌더 객체들이, 버텍스 정보를 각각 갖고 있어야 할까?
		ID3D11Buffer* mVB;
		ID3D11Buffer* mIB;

		ID3DX11Effect* mFX;
		ID3DX11EffectTechnique* mTech;

		// 이건 월드좌표에 대응되는 변환 매트릭스
		ID3DX11EffectMatrixVariable* mfxWorldViewProj;

		ID3D11InputLayout* mInputLayout;

		XMMATRIX mWorld;	// Transform Matrix
		XMMATRIX mView;
		XMMATRIX mProj;

		// 와이어로 그리기 위한 RenderState. 쉐이더에서 해도 된다.
		// 예제에서 On/Off로 바꾸는 등의 일을 하는 경우 이것을 바꿔줘도 될 것 같다.
		// 쉐이더에서 하는 경우도 스위칭 비용도 줄일 수 있는 방법은 많다.
		ID3D11RasterizerState* m_pRenderstate;


	};
}
