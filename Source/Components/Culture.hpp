#pragma once

#include "Province.hpp"
#include "Army.hpp"

enum CultureType {
    SteppeNomads,
    FarmLanders,
    ForestFolk,
    HillDwellers
};

struct CultureTraits {
    UnitType peasant_levies;
    UnitType noble_levies;

    TerrainType preferred_terrain;
    BiomeType preferred_biome;

    bool extra_development;
    bool extra_control;
    bool extra_levies;
    bool extra_movement;
};

CultureTraits GetTraits(CultureType culture) {
    switch (culture) {
        case SteppeNomads:
            return {
                LightCavalry,
                ArcherCavalry,

                Plains,
                Grasslands,

                false,
                false,
                false,
                true
            };

        case FarmLanders:
            return {
                LightInfantry,
                HeavyCavalry,

                Plains,
                Jungles,

                true,
                false,
                false,
                false
            };

        case ForestFolk:
            return {
                ArcherInfantry,
                HeavyInfantry,

                Plains,
                Forests,

                false,
                false,
                true,
                false
            };

        case HillDwellers:
            return {
                LightInfantry,
                LightCavalry,

                Hills,
                Grasslands,

                false,
                true,
                false,
                false
            };
    }
}