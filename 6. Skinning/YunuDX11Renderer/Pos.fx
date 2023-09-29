//***************************************************************************************
// wire.fx by LeHide (C) 2021 All Rights Reserved.
//
// Axes of world, grid���� �׸��� ���� ���̾������ӿ� ���̴�
//***************************************************************************************

cbuffer cbPerObject
{
    float4x4 gWorldViewProj;
    float4 color : COLOR;
};

struct VertexIn
{
    float3 PosL  : POSITION;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to homogeneous clip space.
    // �ٱ��ʿ��� ����-��-�������� ����� �޾Ƽ� ����������ǥ�� (���� NDC�ƴ�)���� ��ȯ���ش�.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    // Just pass vertex color into the pixel shader.
    // ���� ������Ʈ�� ���ؽ��÷��� �����ؼ� �׸��� �Ѵ�.

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return color;
}

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}
