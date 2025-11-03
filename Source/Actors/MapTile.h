#pragma once

#include "Actor.h"

enum TerrainType {
    Wetlands,
    Plains,
    Hills,
    Mountains,

    Water,
};

enum BiomeType {
    Drylands,
    Grasslands,
    Forest,
    Jungle,

    Wasteland,
};

class MapTile : public Actor
{
public:
    explicit MapTile(Game* game, TerrainType terrainType, BiomeType biomeType);
    void OnUpdate(float deltaTime) override;

    [[nodiscard]] TerrainType GetTerrain() const {return mTerrain;};
    [[nodiscard]] BiomeType GetBiome() const {return mBiome;};
protected:
    TerrainType mTerrain;
    BiomeType mBiome;
    AABBColliderComponent* mHitbox;
};