#include "Camera.hpp"

#include <algorithm>
#include <SDL3/SDL.h>
#include "imgui.h"
#include "Window.hpp"
#include "Game.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/norm.inl"

Camera::Camera()
{
    RecalculateView();
}

void Camera::RecalculateView()
{
    mPosition.x = mTarget.x;
    mPosition.y = mTarget.y;
    mPosition.z = 1.0f;

    mView = glm::lookAt(mPosition, glm::vec3(mTarget, 0), mUp);
}

float Camera::GetProjectionScale() const
{
    return mZoomLevel * 0.02f;
}

glm::mat4 Camera::CalculateProjection(const Window& window) const
{
    // Apply zoom to the orthographic size
    const auto &sz = window.GetSize() * GetProjectionScale();
    // const float fov = 2.0f * atan2(sz.y * 0.5f, 1.0f / scale);
    // return glm::perspectiveFov(fov, sz.x, sz.y, 0.1f, 100.0f);
    return glm::ortho(sz.x * -0.5f, sz.x * 0.5f, sz.y * -0.5f, sz.y * 0.5f, 0.1f, 100.0f);
}

glm::vec3 Camera::NDCToWorld(const glm::vec2& NDC, const Window& window) const
{
    const glm::mat4 VPI = glm::inverse(CalculateProjection(window) * mView);
    return VPI * glm::vec4(NDC, 0.0f, 1.0f);
}

void UpdateCamera(Camera &camera, const InputState &input, const Window &window, float deltaTime)
{
    const bool* state = SDL_GetKeyboardState(nullptr);

    // Change Positions
    glm::vec2 &velocity = camera.mVelocity;
    const float shiftModifier = (state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT]) ? 2.0f : 1.0f;
    if (glm::length2(velocity) > 0.0f)
    {
        const float scale = camera.GetProjectionScale() * deltaTime;
        camera.mTarget += glm::vec2{velocity.x, velocity.y} * scale * shiftModifier;
    }
    if (fabsf(camera.mZoomInertia) > 0.0f)
    {
        camera.mZoomLevel += camera.mZoomInertia * deltaTime * shiftModifier;
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
    if (!io.WantCaptureKeyboard)
    {
        const float keyAccelSpeed = camera.mMoveSpeed * deltaTime;
        if (state[SDL_SCANCODE_W]) {
            velocity.y += keyAccelSpeed;
        }
        if (state[SDL_SCANCODE_S]) {
            velocity.y -= keyAccelSpeed;
        }
        if (state[SDL_SCANCODE_A]) {
            velocity.x -= keyAccelSpeed;
        }
        if (state[SDL_SCANCODE_D]) {
            velocity.x += keyAccelSpeed;
        }
    }

    if (!io.WantCaptureMouse)
    {
        const float edgeAccelSpeed = camera.mEdgeScrollSpeed * deltaTime;
        const auto &mousePosNDC = window.GetMousePosNDC();
        if (mousePosNDC.y > 0.99) {
            velocity.y += edgeAccelSpeed;
        }
        if (mousePosNDC.y < -0.99) {
            velocity.y -= edgeAccelSpeed;
        }
        if (mousePosNDC.x < -0.99) {
            velocity.x -= edgeAccelSpeed;
        }
        if (mousePosNDC.x > 0.99) {
            velocity.x += edgeAccelSpeed;
        }

        // Mouse inputs don't take deltaTime because they're in deltas already
        if (input.IsRightMouseButtonDown || input.IsMiddleMouseButtonDown)
        {
            velocity.x = -input.MouseDelta.x * camera.mPanSpeed;
            velocity.y = input.MouseDelta.y * camera.mPanSpeed;
        }
        if (input.MouseScrollAmount != 0.0f)
        {
            camera.mZoomInertia -= input.MouseScrollAmount * camera.mZoomSpeed;
        }
    }
}
