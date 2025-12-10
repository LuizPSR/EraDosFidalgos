#pragma once
#include <cstdint>

struct DiplomacyModule
{
    explicit DiplomacyModule(const flecs::world &ecs);
};

struct RealmRelation
{
    int8_t relations = 0;
};

struct War
{
    uint32_t mStartDate;
};

struct Neighboring {};