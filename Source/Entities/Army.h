#pragma once
#include "Actor.h"
#include "MapTile.h"

enum UnitType {
    LightInfantry,
    ArcherInfantry,
    HeavyInfantry,

    LightCavalry,
    ArcherCavalry,
    HeavyCavalry,
};
class Unit {
public:
    UnitType type;
    unsigned int initialForce;
    unsigned int currentForce;

    MapTile* homeland;
};

class Army {
public:
    explicit Army(Game* game);
    ~Army() override;

    void MoveTo(Vector2 position);
    unsigned int GetCurrentForce() const;
    void Disband();
    void Fight(Army* enemy);

private:
    std::vector<Unit> host;
    MapTile* location;

};
