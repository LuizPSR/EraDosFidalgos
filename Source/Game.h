#pragma once
#include <flecs.h>
#include <glm/glm.hpp>

bool Initialize(flecs::world &ecs);
void ProcessInput(const flecs::world &ecs);
void RegisterSystems(flecs::world &ecs);

struct InputState
{
    bool IsRightMouseButtonDown = false;
    bool IsMiddleMouseButtonDown = false;
    bool WasEscapePressed = false;
    glm::vec2 MouseDelta;
    float MouseScrollAmount = 0.0f;
};
