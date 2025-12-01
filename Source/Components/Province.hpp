#pragma once
#include <string>
#include <cstdint>

#include "Culture.hpp"
#include "Geograph.hpp"

struct InRealm {};

struct Province
{
    std::string name;

    uint64_t mPosX = 0, mPosY = 0;

    uint64_t income = 0;
    uint64_t development = 0;
    uint64_t control = 0;
    int32_t popular_opinion = 0;
    float distance_to_capital = 0;

    CultureType culture = FarmLanders;

    // Buildings
    uint32_t market_level = 0;
    uint32_t temples_level = 0;
    uint32_t roads_level = 0;
    uint32_t barracks_level = 0;
    uint32_t fortification_level = 0;

    // Geography
    TerrainType terrain = TerrainType::Sea;
    BiomeType biome = BiomeType::Water;
    float movement_cost = 0;

    // Transforms income from fixed point to floating point
    [[nodiscard]] double IncomeFloat() const
    {
        return static_cast<double>(income) * 0.01;
    }
};