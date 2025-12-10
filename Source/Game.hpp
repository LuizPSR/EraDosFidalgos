#pragma once
#include <flecs.h>
#include <glm/glm.hpp>

bool Initialize(flecs::world &ecs);
void ProcessInput(const flecs::world &ecs);
void RegisterSystems(flecs::world &ecs);
void ImportModules(flecs::world &ecs);

struct InputState
{
    bool IsRightMouseButtonDown = false;
    bool IsMiddleMouseButtonDown = false;
    bool WasEscapePressed = false;
    bool Clicked = false;
    glm::vec2 MouseDelta;
    float MouseScrollAmount = 0.0f;
    int LastNumber = 1;
};

struct GameTickSources
{
    explicit GameTickSources(const flecs::world &ecs);

    // Timers
    // Disabled on pause
    flecs::timer mTickTimer;
    // Run every day (MUST get amount of days elapsed from GameTime struct)
    flecs::timer mDayTimer;
    flecs::timer mWeekTimer;
    flecs::timer mMonthTimer;
    flecs::timer mYearTimer;
};

struct GameStarted {};
struct GameEnded {};
