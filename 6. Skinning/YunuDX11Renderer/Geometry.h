#pragma once

//***************************************************************************************
// Geometry.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************

#include "d3dUtil.h"

class Geometry
{
public:
    struct Vertex
    {
        Vertex() {}
        Vertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT2& uv)
            : Position(p), Normal(n), TangentU(t), TexC(uv) {}
        Vertex(
            float px, float py, float pz,
            float nx, float ny, float nz,
            float tx, float ty, float tz,
            float u, float v)
            : Position(px, py, pz), Normal(nx, ny, nz),
            TangentU(tx, ty, tz), TexC(u, v) {}

        XMFLOAT3 Position{ 0,0,0 };
        XMFLOAT3 Normal{ 0,0,1 };
        XMFLOAT3 TangentU{ 1,0,0 };
        XMFLOAT2 TexC{ 0,0 };
    };

    struct MeshData
    {
        std::vector<Vertex> Vertices;
        std::vector<UINT> Indices;
    };
    static MeshData MergeMesh(const MeshData& lhs, const MeshData& rhs);

    ///<summary>
    /// Creates a box centered at the origin with the given dimensions.
    ///</summary>
    static void CreateBox(float width, float height, float depth, MeshData& meshData, const XMFLOAT3& pivot = XMFLOAT3(0.5f, 0.5f, 0.5f));

    ///<summary>
    /// Creates a sphere centered at the origin with the given radius.  The
    /// slices and stacks parameters control the degree of tessellation.
    ///</summary>
    static void CreateSphere(float radius, UINT sliceCount, UINT stackCount, MeshData& meshData, const XMFLOAT3& pivot = XMFLOAT3(0.5f, 0.5f, 0.5f));

    ///<summary>
    /// Creates a geosphere centered at the origin with the given radius.  The
    /// depth controls the level of tessellation.
    ///</summary>
    static void CreateGeosphere(float radius, UINT numSubdivisions, MeshData& meshData, const XMFLOAT3& pivot = XMFLOAT3(0.5f, 0.5f, 0.5f));

    ///<summary>
    /// Creates a cylinder parallel to the y-axis, and centered about the origin.  
    /// The bottom and top radius can vary to form various cone shapes rather than true
    // cylinders.  The slices and stacks parameters control the degree of tessellation.
    ///</summary>
    static void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData, const XMFLOAT3& pivot = XMFLOAT3(0.5f, 0.5f, 0.5f));

    ///<summary>
    /// Creates an mxn grid in the xz-plane with m rows and n columns, centered
    /// at the origin with the specified width and depth.
    ///</summary>
    static void CreateGrid(float width, float depth, UINT m, UINT n, MeshData& meshData, const XMFLOAT3& pivot = XMFLOAT3(0.5f, 0.5f, 0.5f));

    ///<summary>
    /// Creates a quad covering the screen in NDC coordinates.  This is useful for
    /// postprocessing effects.
    ///</summary>
    static void CreateFullscreenQuad(MeshData& meshData);

private:
    Geometry() = default;
    static void Subdivide(MeshData& meshData);
    static void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
    static void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
};

