#pragma once
#include <flecs.h>
#include <glm/glm.hpp>

bool Initialize(flecs::world &ecs);
void ProcessInput(const flecs::world &ecs);
void RegisterSystems(flecs::world &ecs);
void ImportModules(flecs::world &ecs);

struct GameTimers
{
    // Disabled on pause
    flecs::timer mTickTimer;
};

struct InputState
{
    bool IsRightMouseButtonDown = false;
    bool IsMiddleMouseButtonDown = false;
    bool WasEscapePressed = false;
    glm::vec2 MouseDelta;
    float MouseScrollAmount = 0.0f;
};
