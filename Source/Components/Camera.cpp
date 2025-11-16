#include "Camera.h"

#include <algorithm>
#include <SDL3/SDL.h>
#include "imgui.h"
#include "../Math.h"
#include "../Game.h"
#include "../Renderer/Renderer.h"

void Camera::RecalculateView()
{
    mPosition.x = mTarget.x;
    mPosition.y = mTarget.y;
    mPosition.z = 1.0f;

    mTarget.z = 0.0f;

    mView = Matrix4::CreateLookAt(mPosition, mTarget, mUp);
}

Matrix4 Camera::CalculateProjection(const struct Renderer& renderer) const
{
    const auto &sz = renderer.GetWindowSize();
    float halfW = sz.x * 0.5f;
    float halfH = sz.y * 0.5f;
    // Apply zoom to the orthographic size
    float left   = -halfW * mZoomLevel * 0.02f;
    float right  =  halfW * mZoomLevel * 0.02f;
    float bottom = -halfH * mZoomLevel * 0.02f;
    float top    =  halfH * mZoomLevel * 0.02f;
    return Matrix4::CreateOrtho(left, right, bottom, top, 0.1f, 100.0f);
}

void UpdateCamera(Camera &camera, const InputState &input, float deltaTime)
{

    const auto &io = ImGui::GetIO();
    if (io.WantCaptureKeyboard || io.WantCaptureMouse)
    {
        return;
    }

    Vector3 velocity = Vector3::Zero;
    const float moveSpeed = camera.mMoveSpeed * deltaTime;
    const bool* state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_W]) {
        velocity.y += 1.0f;
    }
    if (state[SDL_SCANCODE_S]) {
        velocity.y -= 1.0f;
    }
    if (state[SDL_SCANCODE_A]) {
        velocity.x += 1.0f;
    }
    if (state[SDL_SCANCODE_D]) {
        velocity.x -= 1.0f;
    }
    if (velocity.LengthSq() > 0.0f)
    {
        velocity.Normalize();
        camera.mTarget += velocity * moveSpeed;
    }
    if (input.IsRightMouseButtonDown || input.IsMiddleMouseButtonDown)
    {
        camera.mTarget.x += input.MouseDeltaX * moveSpeed * 0.5f;
        camera.mTarget.y += input.MouseDeltaY * moveSpeed * 0.5f;
    }
    if (input.MouseScrollAmount != 0.0f)
    {
        camera.mZoomLevel -= input.MouseScrollAmount * camera.mZoomSpeed;
        camera.mZoomLevel = std::clamp<float>(camera.mZoomLevel, camera.mMinZoom, camera.mMaxZoom);
    }
    camera.RecalculateView();
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
