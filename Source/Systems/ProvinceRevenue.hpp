#pragma once

#include <flecs.h>
#include "Game.hpp"
#include "GameTime.hpp"

void DoProvinceRevenueSystems(const flecs::world& ecs, const GameTickSources &timers);
