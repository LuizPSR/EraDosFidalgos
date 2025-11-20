#include "Camera.h"

#include <algorithm>
#include <SDL3/SDL.h>
#include "imgui.h"
#include "../Math.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"

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

    mView = Matrix4::CreateLookAt(mPosition, mTarget, mUp);
}

float Camera::GetProjectionScale() const
{
    return mZoomLevel * 0.02f;
}

Matrix4 Camera::CalculateProjection(const Renderer& renderer) const
{
    const auto &sz = renderer.GetWindowSize();
    float halfW = sz.x * 0.5f;
    float halfH = sz.y * 0.5f;
    // Apply zoom to the orthographic size
    const float scale = GetProjectionScale();
    float left   = -halfW * scale;
    float right  =  halfW * scale;
    float bottom = -halfH * scale;
    float top    =  halfH * scale;
    return Matrix4::CreateOrtho(left, right, bottom, top, 0.1f, 100.0f);
}

void UpdateCamera(Camera &camera, const InputState &input, float deltaTime)
{
    const bool* state = SDL_GetKeyboardState(nullptr);

    // Change Positions
    Vector2 &velocity = camera.mVelocity;
    const float scale = camera.GetProjectionScale() * deltaTime;
    if (velocity.LengthSq() > 0.0f)
    {
        if (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT])
        {
            camera.mTarget += Vector3{velocity.x, velocity.y, 0.0f} * scale  * 2.0f;
        } else
        {
            camera.mTarget += Vector3{velocity.x, velocity.y, 0.0f} * scale;
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
    if (camera.mVelocity.LengthSq() < 0.01f)
        camera.mVelocity = Vector2::Zero;
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
        velocity.x += accelSpeed;
    }
    if (state[SDL_SCANCODE_D]) {
        velocity.x -= accelSpeed;
    }

    // Mouse inputs don't take deltaTime because they're in deltas already
    if (input.IsRightMouseButtonDown || input.IsMiddleMouseButtonDown)
    {
        velocity.x = input.MouseDeltaX * camera.mMoveSpeed * 0.05f;
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
