#pragma once
#include <flecs.h>
#include <glm/glm.hpp>

bool Initialize(const flecs::world &ecs);
void ProcessInput(const flecs::world &ecs);
void RegisterSystems(const flecs::world &ecs);

struct InputState
{
    bool IsRightMouseButtonDown = false;
    bool IsMiddleMouseButtonDown = false;
    glm::vec2 MouseDelta;
    float MouseScrollAmount = 0.0f;
};
