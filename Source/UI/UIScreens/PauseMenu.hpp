#pragma once
#include <flecs.h>
#include "Game.hpp"

struct PauseMenuModule {
    explicit PauseMenuModule(flecs::world &ecs);
    
    static void ShowPauseMenu(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input);
};