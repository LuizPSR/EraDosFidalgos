#pragma once
#include <flecs.h>

struct ArmyModule
{
    explicit ArmyModule(const flecs::world &ecs);
};

struct ProvinceArmy
{
    uint32_t mAmount;
};

struct MovingArmies
{
    flecs::entity mProvince;
    int mAmount;
};
