//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"
#include "PosEffect.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
    : mFX(0)
{
    std::ifstream fin(filename, std::ios::binary);

    fin.seekg(0, std::ios_base::end);
    int size = (int)fin.tellg();
    fin.seekg(0, std::ios_base::beg);
    std::vector<char> compiledShader(size);

    fin.read(&compiledShader[0], size);
    fin.close();

    HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
        0, device, &mFX));
}

Effect::~Effect()
{
    mFX->Release();
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
    : Effect(device, filename)
{
    Light1Tech = mFX->GetTechniqueByName("Light1");
    Light2Tech = mFX->GetTechniqueByName("Light2");
    Light3Tech = mFX->GetTechniqueByName("Light3");
    WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
    World = mFX->GetVariableByName("gWorld")->AsMatrix();
    WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
    EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
    DirLights = mFX->GetVariableByName("gDirLights");
    PointLights = mFX->GetVariableByName("gPointLights");
    SpotLights = mFX->GetVariableByName("gSpotLights");
    Mat = mFX->GetVariableByName("gMaterial");


    /// ÅØ½ºÃÄ
    Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
    Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
    Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
    Light3TexTech = mFX->GetTechniqueByName("Light3Tex");
    DebugNormalTech = mFX->GetTechniqueByName("DebugNormal");

    TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
    DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = 0;
BasicEffect* Effects::BasicTexFX = 0;
PosEffect* Effects::Pos = 0;


void Effects::InitAll(ID3D11Device* device)
{
    BasicFX = new BasicEffect(device, L"../Win32/Debug/Basic.cso");
    BasicTexFX = new BasicEffect(device, L"../Win32/Debug/BasicTex.cso");
    Pos = new PosEffect(device, L"../Win32/Debug/Pos.cso");

    //BasicFX = new BasicEffect(device, L"Basic.cso");
    //BasicTexFX = new BasicEffect(device, L"BasicTex.cso");
    //Pos = new PosEffect(device, L"Pos.cso");
}

void Effects::DestroyAll()
{
    SafeDelete(BasicFX);
    SafeDelete(BasicTexFX);
    SafeDelete(Pos);
}
#pragma endregion