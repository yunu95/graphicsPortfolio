#include "PosEffect.h"

#pragma region PosEffect
PosEffect::PosEffect(ID3D11Device* device, const std::wstring& filename)
    : Effect(device, filename)
{
    colorTech = mFX->GetTechniqueByName("ColorTech");
    WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
    World = mFX->GetVariableByName("gWorld")->AsMatrix();
    WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
    Color = mFX->GetVariableByName("color");
}

PosEffect::~PosEffect()
{
}
#pragma endregion