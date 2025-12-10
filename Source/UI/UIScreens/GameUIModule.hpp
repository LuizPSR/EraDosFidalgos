#pragma once
#include <flecs.h>
#include "Game.hpp"

struct GameUIModule {
    explicit GameUIModule(flecs::world &ecs);

    static void ShowTestUI(const flecs::world& ecs, GameTickSources& tickSources);
};