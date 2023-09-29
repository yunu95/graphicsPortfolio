#pragma once

//***************************************************************************************
// Vertex.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines vertex structures and input layouts.
//***************************************************************************************


#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

namespace VertexStruct
{
	struct PosNormal
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
	};
	struct Pos
	{
		XMFLOAT3 Pos;
	};

	/// 텍스쳐 예제로 추가. 이름은 용책과의 연관성을 위해서 그냥 유지함
	struct Basic32
	{
		XMFLOAT3 Pos;
		XMFLOAT3 Normal;
		XMFLOAT2 Tex;
	};
}
