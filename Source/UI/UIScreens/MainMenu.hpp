#pragma once
#include <flecs.h>
#include "Game.hpp"

struct MainMenuModule {
    explicit MainMenuModule(flecs::world &ecs);
    
    static void ShowMainMenu(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input);
};