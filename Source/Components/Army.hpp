#pragma once

enum UnitType {
    LightInfantry,
    ArcherInfantry,
    HeavyInfantry,

    LightCavalry,
    ArcherCavalry,
    HeavyCavalry,
};


struct Unit {
    UnitType type;
    unsigned int initialForce;
    unsigned int currentForce;
};

