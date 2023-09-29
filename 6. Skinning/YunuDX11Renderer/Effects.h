#pragma once

//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************


#include <array>
#include <concepts>
#include "d3dUtil.h"
#include "d3dx11Effect.h"	// effect, tech

template<typename Container, typename ValueType>
concept ContiguousRange =
std::ranges::range<Container> &&
std::is_same_v<std::ranges::range_value_t<Container>, ValueType> &&
std::contiguous_iterator<std::ranges::iterator_t<Container>>;

class PosEffect;

#pragma region Effect
class Effect
{
public:
    Effect(ID3D11Device* device, const std::wstring& filename);
    virtual ~Effect();

private:
    Effect(const Effect& rhs);
    Effect& operator=(const Effect& rhs);

protected:
    ID3DX11Effect* mFX;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
    BasicEffect(ID3D11Device* device, const std::wstring& filename);
    virtual ~BasicEffect();

    void SetWorldViewProj(CXMMATRIX M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorld(CXMMATRIX M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorldInvTranspose(CXMMATRIX M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetEyePosW(const XMFLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
    void SetDirLights(const DirectionalLight* lights) { DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
    void SetPointLights(const PointLight* lights) { PointLights->SetRawValue(lights, 0, 3 * sizeof(PointLight)); }
    void SetSpotLights(const SpotLight* lights) { SpotLights->SetRawValue(lights, 0, 3 * sizeof(SpotLight)); }
    void SetMaterial(const Material& mat) { Mat->SetRawValue(&mat, 0, sizeof(Material)); }

    /// 텍스쳐 추가
    void SetTexTransform(CXMMATRIX M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }


    ID3DX11EffectTechnique* Light1Tech;
    ID3DX11EffectTechnique* Light2Tech;
    ID3DX11EffectTechnique* Light3Tech;

    ID3DX11EffectMatrixVariable* WorldViewProj;
    ID3DX11EffectMatrixVariable* World;
    ID3DX11EffectMatrixVariable* WorldInvTranspose;
    ID3DX11EffectVectorVariable* EyePosW;
    ID3DX11EffectVariable* DirLights;
    ID3DX11EffectVariable* PointLights;
    ID3DX11EffectVariable* SpotLights;
    ID3DX11EffectVariable* Mat;


    /// 텍스쳐 추가
    ID3DX11EffectTechnique* Light0TexTech;
    ID3DX11EffectTechnique* Light1TexTech;
    ID3DX11EffectTechnique* Light2TexTech;
    ID3DX11EffectTechnique* Light3TexTech;
    ID3DX11EffectTechnique* DebugNormalTech;

    ID3DX11EffectMatrixVariable* TexTransform;
    ID3DX11EffectShaderResourceVariable* DiffuseMap;
};
#pragma endregion

class SkinnedBasicEffect : public BasicEffect
{
public:
    SkinnedBasicEffect(ID3D11Device* device, const std::wstring& filename);
    //virtual ~SkinnedBasicEffect();
    ID3DX11EffectMatrixVariable* bones{ nullptr };

    template<typename Container>
        requires ContiguousRange<Container, XMMATRIX>&&
        requires (const Container& c) { {c.data()}->std::same_as<const XMMATRIX*>; }&&
        requires (const Container& c) { {c.size()}->std::same_as<size_t>; }
    void SetBoneFinalMatrix(const Container& matrixContainer)
    {
        assert(matrixContainer.size() <= 96);
        bones->SetRawValue(matrixContainer.data(), 0, matrixContainer.size() * sizeof(XMMATRIX));
    }
    void SetBoneFinalMatrix2(const std::vector<XMMATRIX>& matrixContainer)
    {
        assert(matrixContainer.size() <= 96);
        //bones->SetRawValue(&matrixContainer[0], 0, matrixContainer.size() * sizeof(XMMATRIX));
        //bones->setmatrixarr(&matrixContainer[0], 0 * sizeof(XMMATRIX), sizeof(XMMATRIX));
        bones->SetMatrixArray(reinterpret_cast<const float*>(&matrixContainer.data()[0]), 0, matrixContainer.size());
    }
};

#pragma region Effects
class Effects
{
public:
    static void InitAll(ID3D11Device* device);
    static void DestroyAll();

    static BasicEffect* BasicFX;
    static BasicEffect* BasicTexFX;
    static SkinnedBasicEffect* skinnedBasicTexFX;
    static PosEffect* Pos;

};
#pragma endregion
