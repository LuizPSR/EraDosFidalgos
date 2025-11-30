//
// Created by thiago on 30/11/2025.
//

#ifndef ERADOSFIDALGOS_TESTUI_H
#define ERADOSFIDALGOS_TESTUI_H


#pragma once
#include <flecs.h>
#include "Game.hpp"

struct TestUIModule {
    explicit TestUIModule(flecs::world &ecs);

    static void ShowTestUI(const flecs::world& ecs, GameTickSources& timers);
};

#endif //ERADOSFIDALGOS_TESTUI_H