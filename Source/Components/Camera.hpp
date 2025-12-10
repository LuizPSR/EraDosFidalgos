#pragma once
#include <glm/glm.hpp>
#include "Window.hpp"

struct Camera
{
    Camera();

    const float mMinZoom = 1.0f;
    const float mMaxZoom = 50.0f;
    const float mZoomSpeed = 16.0f;
    const float mMoveSpeed = 3072.0f;
    const float mPanSpeed = 192.0f;
    const float mEdgeScrollSpeed = 4096.0f;

    glm::mat4 mView{};

    glm::vec2 mVelocity{};
    glm::vec3 mPosition{};
    glm::vec2 mTarget{};
    const glm::vec3 mUp{0.0f, 1.0f, 0.0f};

    float mZoomLevel = 50.0f;
    float mZoomInertia = 0.0f;

    void RecalculateView();
    float GetProjectionScale() const;
    glm::mat4 CalculateProjection(const Window &window) const;
    glm::vec3 NDCToWorld(const glm::vec2& NDC, const Window& window) const;
};

void UpdateCamera(Camera &camera, const struct InputState &input, const Window &window, float deltaTime);
