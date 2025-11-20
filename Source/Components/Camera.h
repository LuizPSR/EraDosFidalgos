#pragma once
#include "../Math.h"
#include "flecs.h"

struct Camera
{
    Camera();

    const float mMinZoom = 1.0f;
    const float mMaxZoom = 50.0f;
    const float mZoomSpeed = 16.0f;
    const float mMoveSpeed = 1024.0f;

    Matrix4 mView;

    Vector2 mVelocity;
    Vector3 mPosition;
    Vector3 mTarget;
    const Vector3 mUp{0.0f, 1.0f, 0.0f};

    float mZoomLevel = 10.0f;
    float mZoomInertia = 0.0f;

    float mYaw = 0.0f;
    float mPitch = -M_PI / 4.0f;

    void RecalculateView();
    Matrix4 CalculateProjection(const struct Renderer &renderer) const;
};

void UpdateCamera(flecs::iter& it);