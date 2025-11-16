#pragma once
#include <flecs.h>

bool Initialize(const flecs::world &ecs);
void ProcessInput(const flecs::world &ecs);
void RegisterSystems(const flecs::world &ecs);

struct InputState
{
    bool IsRightMouseButtonDown = false;
    bool IsMiddleMouseButtonDown = false;
    float MouseDeltaX = 0.0f;
    float MouseDeltaY = 0.0f;
    float MouseScrollAmount = 0.0f;
};
