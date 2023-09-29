#include "Effects.h"
#pragma once
#pragma region PosEffect

class PosEffect : public Effect
{
public:
    PosEffect(ID3D11Device* device, const std::wstring& filename);
    ~PosEffect();

    void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetColor(XMFLOAT4 color) { Color->SetRawValue(&color, 0, sizeof(XMFLOAT4)); }
    void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }

    ID3DX11EffectTechnique* colorTech;

    ID3DX11EffectVariable* Color;
    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
};
#pragma endregion
