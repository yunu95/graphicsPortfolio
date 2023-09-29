#pragma once

#include "d3dUtil.h"

/// <summary>
/// 카메라 클래스. DX11 종속적
/// </summary>
class Camera
{
public:
    Camera();
    ~Camera();

    // Get/Set world camera position.
    XMVECTOR GetPositionXM() const;
    XMFLOAT3 GetPosition() const;
    void SetPosition(float x, float y, float z);
    void SetPosition(const XMFLOAT3& v);

    // Get camera basis vectors.
    XMVECTOR GetRightXM() const;
    XMFLOAT3 GetRight() const;
    XMVECTOR GetUpXM() const;
    XMFLOAT3 GetUp() const;
    XMVECTOR GetLookXM() const;
    XMFLOAT3 GetLook() const;

    // Get frustum properties.
    // 절두체의 형태는 near 평면과의 z 거리, far 평면과의 z거리, 해상도, FOV값에 따라 결정된다.
    float GetNearZ() const;
    float GetFarZ() const;
    float GetAspect() const;
    float GetFovY() const;
    float GetFovX() const;

    // Get near and far plane dimensions in view space coordinates.
    float GetNearWindowWidth() const;
    float GetNearWindowHeight() const;
    float GetFarWindowWidth() const;
    float GetFarWindowHeight() const;

    // Set frustum.
    // 렌더러에서 카메라 절두체의 요소를 바꾸는 것이 현실세계에서 렌즈를 설정하는 것과 같나 보다.
    void SetLens(float fovY, float aspect, float zn, float zf);

    // Define camera space via LookAt parameters.
    // FXMVECTOR와 XMFLOAT3는 서로 다르다.
    void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
    void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

    // Get View/Proj matrices.
    XMMATRIX View() const;
    XMMATRIX Proj() const;
    XMMATRIX ViewProj() const;

    // Strafe/Walk the camera a distance d.
    void Strafe(float d);
    void Walk(float d);
    void WorldUpDown(float d);

    // Rotate the camera.
    void Pitch(float angle);
    void RotateY(float angle);

    // After modifying camera position/orientation, call to rebuild the view matrix.
    void UpdateViewMatrix();

private:
    // Camera coordinate system with coordinates relative to world space.
    XMFLOAT3 mPosition;
    XMFLOAT3 mRight;
    XMFLOAT3 mUp;
    XMFLOAT3 mLook;

    // Cache frustum properties.
    float mNearZ;
    float mFarZ;
    float mAspect;
    float mFovY;
    float mNearWindowHeight;
    float mFarWindowHeight;

    // Cache View/Proj matrices.
    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;
};
