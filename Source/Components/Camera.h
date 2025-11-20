#pragma once
#include <glm/glm.hpp>
#include "flecs.h"

struct Camera
{
    Camera();

    const float mMinZoom = 1.0f;
    const float mMaxZoom = 50.0f;
    const float mZoomSpeed = 16.0f;
    const float mMoveSpeed = 2848.0f;

    glm::mat4 mView;

    glm::vec2 mVelocity;
    glm::vec3 mPosition;
    glm::vec3 mTarget;
    const glm::vec3 mUp{0.0f, 1.0f, 0.0f};

    float mZoomLevel = 10.0f;
    float mZoomInertia = 0.0f;

    float mYaw = 0.0f;
    float mPitch = -M_PI / 4.0f;

    void RecalculateView();
    float GetProjectionScale() const;
    glm::mat4 CalculateProjection(const struct Renderer &renderer) const;

    glm::vec3 NDCToWorld(const glm::vec2 &NDC, const struct Renderer &renderer) const;
};

void UpdateCamera(flecs::iter& it);