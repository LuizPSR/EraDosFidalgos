#include "Camera.h"

#include <algorithm>
#include <SDL3/SDL.h>
#include "imgui.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/norm.inl"

Camera::Camera()
    : mPosition{50.0f, 50.0f, 0.0f},
        mTarget{0.0f, 0.0f, 0.0f},
        mZoomLevel(50.0f)
{
    RecalculateView();
}

void Camera::RecalculateView()
{
    mPosition.x = mTarget.x;
    mPosition.y = mTarget.y;
    mPosition.z = 1.0f;

    mTarget.z = 0.0f;

    mView = glm::lookAt(mPosition, mTarget, mUp);
}

float Camera::GetProjectionScale() const
{
    return mZoomLevel * 0.02f;
}

glm::mat4 Camera::CalculateProjection(const Renderer& renderer) const
{
    // Apply zoom to the orthographic size
    const auto &sz = renderer.GetWindowSize() * GetProjectionScale();
    // const float fov = 2.0f * atan2(sz.y * 0.5f, 1.0f / scale);
    // return glm::perspectiveFov(fov, sz.x, sz.y, 0.1f, 100.0f);
    return glm::ortho(sz.x * -0.5f, sz.x * 0.5f, sz.y * -0.5f, sz.y * 0.5f, 0.1f, 100.0f);
}

glm::vec3 Camera::NDCToWorld(const glm::vec2& NDC, const struct Renderer& renderer) const
{
    const glm::mat4 VPI = glm::inverse(CalculateProjection(renderer) * mView);
    return VPI * glm::vec4(NDC, 0.0f, 1.0f);
}

void UpdateCamera(Camera &camera, const InputState &input, float deltaTime)
{
    const bool* state = SDL_GetKeyboardState(nullptr);

    // Change Positions
    glm::vec2 &velocity = camera.mVelocity;
    const float scale = camera.GetProjectionScale() * deltaTime;
    if (glm::length2(velocity) > 0.0f)
    {
        if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
        {
            camera.mTarget += glm::vec3{velocity.x, velocity.y, 0.0f} * scale  * 2.0f;
        } else
        {
            camera.mTarget += glm::vec3{velocity.x, velocity.y, 0.0f} * scale;
        }
    }
    if (fabsf(camera.mZoomInertia) > 0.0f)
    {
        camera.mZoomLevel += camera.mZoomInertia * deltaTime;
        camera.mZoomLevel = std::clamp<float>(camera.mZoomLevel, camera.mMinZoom, camera.mMaxZoom);
    }

    // Reduce Velocities
    float exponential = powf(0.001f, deltaTime);
    camera.mVelocity *= exponential;
    camera.mZoomInertia *= exponential;
    if (glm::length2(camera.mVelocity) < 0.01f)
        camera.mVelocity = glm::vec2(0.0f);
    if (fabsf(camera.mZoomInertia) < 0.01f)
        camera.mZoomInertia = 0.0f;

    // Recalculate view after moving
    camera.RecalculateView();

    // Receive Commands
    const auto &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantCaptureMouse)
    {
        return;
    }

    const float accelSpeed = camera.mMoveSpeed * deltaTime;
    if (state[SDL_SCANCODE_W]) {
        velocity.y += accelSpeed;
    }
    if (state[SDL_SCANCODE_S]) {
        velocity.y -= accelSpeed;
    }
    if (state[SDL_SCANCODE_A]) {
        velocity.x -= accelSpeed;
    }
    if (state[SDL_SCANCODE_D]) {
        velocity.x += accelSpeed;
    }

    // Mouse inputs don't take deltaTime because they're in deltas already
    if (input.IsRightMouseButtonDown || input.IsMiddleMouseButtonDown)
    {
        velocity.x = input.MouseDeltaX * camera.mMoveSpeed * -0.05f;
        velocity.y = input.MouseDeltaY * camera.mMoveSpeed * 0.05f;
    }
    if (input.MouseScrollAmount != 0.0f)
    {
        camera.mZoomInertia -= input.MouseScrollAmount * camera.mZoomSpeed;
    }
}

void UpdateCamera(flecs::iter& it)
{
    const float deltaTime = it.delta_time();
    while (it.next()) {
        for (auto i : it) {
            auto &camera = it.field_at<Camera>(0, i);
            auto &input = it.field_at<const InputState>(1, i);
            UpdateCamera(camera, input, deltaTime);
        }
    }
}
