#include "ProvinceUpdate.hpp"

#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "Characters.hpp"
#include "GameTime.hpp"
#include "Components/Province.hpp"
#include "EstatePower.hpp"
#include "MapGenerator.hpp"
#include "Random.hpp"

const int ESTATE_EFFECT_THRESHOLD = 75;

// Define a hash for entities to use in unordered_maps
struct EntityHash {
    size_t operator()(const flecs::entity& e) const {
        return e.id();
    }
};

void GatherProvinceRevenue(const flecs::world& ecs, const GameTickSources &timers)
{
    auto qProvinceRuler = ecs.query_builder<Character>("qProvinceRuler")
        .with<RuledBy>("$this").src("$title")
        .with<InRealm>("$title").src("$province")
        .build();

    ecs.system<Province, const GameTime>()
        .tick_source(timers.mYearTimer)
        .each([=](flecs::iter &it, size_t i, Province &province, const GameTime &gameTime)
        {
            qProvinceRuler
                .set_var("province", it.entity(i))
                .each([&](Character &character)
                {
                    character.mMoney += province.income;
                });

            // TODO: pay taxes to higher lieges
        });
}

void UpdateDistanceToCapital(const flecs::world& ecs, const GameTickSources &timers) {
    // We need the TileMap to traverse the grid
    const auto tileMapEntity = ecs.lookup("TileMap");
    if (!tileMapEntity.is_valid()) return;
    const auto& tileMap = tileMapEntity.get<TileMap>();

    // Query all Titles (Kingdoms, Duchies, etc.)
    const auto qTitles = ecs.query<const Title>();

    ecs.system<>("ReCalculateDistancesToCapital")
        .kind(flecs::PreUpdate)
        .tick_source(timers.mMonthTimer) // Update yearly or on demand is usually sufficient
        .run([=](flecs::iter& it) {
            const auto& world = it.world();

            qTitles.each([&](flecs::entity titleEntity, const Title& title) {
                // 1. Find the capital of this title
                // The capital has the relation (CapitalOf, TitleEntity)
                const flecs::entity capitalEntity = world
                    .target<CapitalOf>(titleEntity);

                if (!capitalEntity.is_valid()) return;

                // 2. Prepare Dijkstra
                struct Node {
                    flecs::entity entity;
                    float cost;
                    bool operator>(const Node& other) const { return cost > other.cost; }
                };

                std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
                std::unordered_map<flecs::entity, float, EntityHash> dist;

                // Initialize with Capital
                pq.push({ capitalEntity, 0.0f });
                dist[capitalEntity] = 0.0f;

                // We want to update all provinces in the realm.
                // We don't stop strictly when all are found because pathing might improve,
                // but for performance in a grid, standard Dijkstra is fine.
                // Note: The path can go OUTSIDE the realm, so we iterate the whole graph (limited by max relevant distance or map bounds).

                while (!pq.empty()) {
                    Node current = pq.top();
                    pq.pop();

                    if (current.cost > dist[current.entity]) continue;

                    const auto pPos = current.entity.get<TileData>();
                    const auto pProv = current.entity.get_mut<Province>();


                    // Neighbors
                    const int dx[] = { 0, 0, 1, -1 };
                    const int dy[] = { 1, -1, 0, 0 };

                    // Cost to leave current tile (as per requirement: "cost ... is movement_cost of tile A")
                    float traverseCost = pProv.movement_cost;

                    for (int i = 0; i < 4; ++i) {
                        int nx = pPos.x + dx[i];
                        int ny = pPos.y + dy[i];

                        if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;

                        flecs::entity neighbor = tileMap.tiles[nx][ny];
                        if (!neighbor.is_valid()) continue;

                        // Skip if Sea (unless ships are implemented, usually standard expansion blocks on sea)
                        // Assuming movement logic similar to CreateKingdoms
                        const auto nProv = neighbor.get_mut<Province>();
                        if (nProv.terrain == TerrainType::Sea) continue;

                        float newDist = current.cost + traverseCost;

                        if (!dist.count(neighbor) || newDist < dist[neighbor]) {
                            dist[neighbor] = newDist;
                            pq.push({ neighbor, newDist });
                        }
                    }
                }

                // 3. Update Provinces belonging to this Title
                // Iterate all provinces that are 'InRealm' of this Title
                world.query_builder<Province>()
                    .with<InRealm>(titleEntity)
                    .each([&](flecs::entity pEntity, Province& p) {
                        if (dist.count(pEntity)) {
                            p.distance_to_capital = dist[pEntity];
                        } else {
                            // If unreachable
                            p.distance_to_capital = 9999.0f;
                        }
                    });
            });
        });
}

void UpdateStats(const flecs::world& ecs, const GameTickSources &timers) {
    ecs.system<>("EstateEffects")
        .kind(flecs::PreUpdate)
        .tick_source(timers.mWeekTimer)
        .run([=](flecs::iter& it) {
            const auto estates = it.world().get<EstatePowers>();

            auto provinces = std::vector<Province>{};

            auto qPlayerProvinces = ecs.query_builder<const Province>()
                .with<InRealm>("$title")
                .with<RulerOf>("$title").src<Player>()
                .with<Player>().src("$player")
                .build();

            qPlayerProvinces.each([&](flecs::entity t, const Province& p) {
                    provinces.push_back(p);
            });

            // Commoners Power

            int commoners = estates.mCommonersPower;
            int change = 0;
            int points = 0;

            if (commoners > ESTATE_EFFECT_THRESHOLD) {
                change = 1;
                points = commoners - ESTATE_EFFECT_THRESHOLD;
            } else if (commoners < -ESTATE_EFFECT_THRESHOLD) {
                change = -1;
                points = std::abs(commoners) - ESTATE_EFFECT_THRESHOLD;
            }

            while (points > 0) {
                points--;

                auto index = Random::GetIntRange(0, provinces.size());
                auto p = provinces.at(index);

                p.development += change;
            }

            // Nobles and Clergy Power

            float nobilityMod = 0.0f;
            float clergyMod = 0.0f;

            // Clergy -> Control (Scale 0 to +/- 25)
            if (estates.mNobilityPower > ESTATE_EFFECT_THRESHOLD) {
                nobilityMod = (estates.mNobilityPower - ESTATE_EFFECT_THRESHOLD);
            } else if (estates.mNobilityPower < -ESTATE_EFFECT_THRESHOLD) {
                nobilityMod = (estates.mNobilityPower + ESTATE_EFFECT_THRESHOLD); // Negative result
            }

            // Clergy -> Popular Opinion (Scale 0 to +/- 25)
            if (estates.mClergyPower > ESTATE_EFFECT_THRESHOLD) {
                clergyMod = (estates.mClergyPower - ESTATE_EFFECT_THRESHOLD);
            } else if (estates.mClergyPower < -ESTATE_EFFECT_THRESHOLD) {
                clergyMod = (estates.mClergyPower + ESTATE_EFFECT_THRESHOLD);
            }

            ecs.query_builder<CharacterCulture>()
                .with<Player>()
                .build()
                .each([&](flecs::entity t, CharacterCulture& culture) {
                    auto rulerCulture = culture.culture;
                    auto rulerTraits = GetCulturalTraits(rulerCulture);

                    for (auto p: provinces) {

                        p.popular_opinion =
                            std::clamp(
                                5.f * p.temples_level
                                + clergyMod
                                - 25 * (p.culture != rulerCulture)

                                , 0.0f, 100.0f
                            );



                        p.control =
                            std::clamp(
                                100
                                + 10 * p.fortification_level
                                + 10 * (rulerTraits.extra_control)
                                + nobilityMod
                                - p.distance_to_capital
                                + p.popular_opinion * (p.popular_opinion < 0)

                                , 0.f, 100.f
                            );

                        p.movement_cost = 30
                            +  15 * (p.terrain == Plains)
                            +  15 * (p.terrain == Mountains)
                            +  15 * (p.roads_level == 0 && (p.biome == Forests || p.biome == Jungles))
                            -  5 * p.roads_level;
                    }
                });
        });
}

ProvinceUpdates::ProvinceUpdates(flecs::world &ecs) {
    const auto &timers = ecs.get<GameTickSources>();

    GatherProvinceRevenue(ecs, timers);
    UpdateDistanceToCapital(ecs, timers);
    UpdateStats(ecs, timers);

}
