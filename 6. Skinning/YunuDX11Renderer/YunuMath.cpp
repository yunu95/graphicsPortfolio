//***************************************************************************************
// based on YunuMath code by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "YunuMath.h"
#include <float.h>
#include <cmath>


float YunuMath::RandF()
{
    return (float)(rand()) / (float)RAND_MAX;
}

// Returns random float in [a, b).
float YunuMath::RandF(float a, float b)
{
    return a + RandF() * (b - a);
}
constexpr long double YunuMath::operator"" _degree(long double deg)
{
    return deg * YunuMath::Pi / 180.0;
}
float YunuMath::AngleFromXY(float x, float y)
{
    float theta = 0.0f;

    // Quadrant I or IV
    if (x >= 0.0f)
    {
        // If x = 0, then atanf(y/x) = +pi/2 if y > 0
        //                atanf(y/x) = -pi/2 if y < 0
        theta = atanf(y / x); // in [-pi/2, +pi/2]

        if (theta < 0.0f)
            theta += 2.0f * Pi; // in [0, 2*pi).
    }

    // Quadrant II or III
    else
        theta = atanf(y / x) + Pi; // in [0, 2*pi).

    return theta;
}

XMMATRIX YunuMath::InverseTranspose(CXMMATRIX M)
{
    // Inverse-transpose is just applied to normals.  So zero out 
    // translation row so that it doesn't get into our inverse-transpose
    // calculation--we don't want the inverse-transpose of the translation.
    XMMATRIX A = M;
    //A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    XMVECTOR det = XMMatrixDeterminant(A);
    A = XMMatrixInverse(&det, A);
    return XMMatrixTranspose(A);
    //return XMMatrixTranspose(XMMatrixInverse(&det, A));
}
XMVECTOR YunuMath::RandUnitVec3()
{
    XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
    XMVECTOR Zero = XMVectorZero();

    // Keep trying until we get a point on/in the hemisphere.
    while (true)
    {
        // Generate random point in the cube [-1,1]^3.
        XMVECTOR v = XMVectorSet(YunuMath::RandF(-1.0f, 1.0f), YunuMath::RandF(-1.0f, 1.0f), YunuMath::RandF(-1.0f, 1.0f), 0.0f);

        // Ignore points outside the unit sphere in order to get an even distribution 
        // over the unit sphere.  Otherwise points will clump more on the sphere near 
        // the corners of the cube.

        if (XMVector3Greater(XMVector3LengthSq(v), One))
            continue;

        return XMVector3Normalize(v);
    }
}

XMVECTOR YunuMath::RandHemisphereUnitVec3(XMVECTOR n)
{
    XMVECTOR One = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
    XMVECTOR Zero = XMVectorZero();

    // Keep trying until we get a point on/in the hemisphere.
    while (true)
    {
        // Generate random point in the cube [-1,1]^3.
        XMVECTOR v = XMVectorSet(YunuMath::RandF(-1.0f, 1.0f), YunuMath::RandF(-1.0f, 1.0f), YunuMath::RandF(-1.0f, 1.0f), 0.0f);

        // Ignore points outside the unit sphere in order to get an even distribution 
        // over the unit sphere.  Otherwise points will clump more on the sphere near 
        // the corners of the cube.

        if (XMVector3Greater(XMVector3LengthSq(v), One))
            continue;

        // Ignore points in the bottom hemisphere.
        if (XMVector3Less(XMVector3Dot(n, v), Zero))
            continue;

        return XMVector3Normalize(v);
    }
}
