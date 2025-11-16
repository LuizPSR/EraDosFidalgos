#pragma once
#include "../Math.h"
#include "flecs.h"

struct Camera
{
    const float mMinZoom = 1.0f;
    const float mMaxZoom = 50.0f;
    const float mZoomSpeed = 2.0f;
    const float mMoveSpeed = 64.0f;

    Matrix4 mView;

    Vector3 mPosition;
    Vector3 mTarget;
    const Vector3 mUp{0.0f, 1.0f, 0.0f};

    float mZoomLevel = 10.0f;

    float mYaw = 0.0f;
    float mPitch = -M_PI / 4.0f;

    void RecalculateView();
    Matrix4 CalculateProjection(const struct Renderer &renderer) const;
};

void UpdateCamera(flecs::iter& it);