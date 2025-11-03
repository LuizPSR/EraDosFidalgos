#pragma once
#include "Actor.h"
#include "MapTile.h"

enum SettlementType {
    Rural,
    Town,
    City,
};

class Settlement : public Actor {
public:
    explicit Settlement(Game* game, SettlementType settlementType, MapTile* location);
    void OnUpdate(float deltaTime) override;
    [[nodiscard]] SettlementType GetType() const {return mType;};

private:
    SettlementType mType;
    MapTile* mLocation;

    unsigned int mDevelopment = 0;
    unsigned int mControl = 0;

    bool mLeviesRaised = false;
};