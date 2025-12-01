#pragma once
#include <string>
#include <cstdint>

struct InRealm {};

enum TerrainType {
    Sea,

    Wetlands,
    Plains,
    Hills,
    Mountains,
};

enum BiomeType {
    Water,

    Drylands,    // hot  and dry
    Grasslands,  // cold and dry
    Jungles,     // hot  and wet
    Forests,     // cold and dry
};

struct Province
{
    std::string name;

    uint64_t mPosX = 0, mPosY = 0;

    uint64_t income = 0;
    uint64_t development = 0;
    uint64_t control = 0;
    int32_t popular_opinion = 0;
    float distance_to_capital = 0;

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

    // Transforms income from fixed point to floating point (USE FOR DISPLAY ONLY)
    [[nodiscard]] double IncomeFloat() const
    {
        return static_cast<double>(income) * 0.01;
    }
};