#pragma once
#include <cstdint>

struct DiplomacyModule
{
    explicit DiplomacyModule(const flecs::world &ecs);
};

struct RealmRelation
{
    uint8_t diplomacy;
};
