#pragma once
#include <flecs.h>
#include "Game.hpp"

struct TestUIModule {
    explicit TestUIModule(flecs::world &ecs);

    static void ShowTestUI(const flecs::world& ecs, GameTickSources& tickSources);
};