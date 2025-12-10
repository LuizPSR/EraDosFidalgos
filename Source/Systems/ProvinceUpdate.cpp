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
                    province.income = // base value
                        10
                        + 3 * province.market_level
                        - 2 * province.temples_level
                        - 2 * province.fortification_level
                        - province.roads_level;

                    province.income =
                        province.income
                        * (province.development + 100)
                        * province.control * 0.01;

                    character.mMoney += province.income;
                });
        });
}

void UpdateDistanceToCapital(const flecs::world& ecs, const GameTickSources &timers) {
    // Iterate over the TileMap to access the grid structure and global dimensions
    ecs.system<const TileMap>("ReCalculateDistancesToCapital")
        .kind(flecs::PreUpdate)
        .tick_source(timers.mMonthTimer)
        .each([&](flecs::iter& it, size_t, const TileMap &tileMap) {

            auto world = it.world();

            // 1. Query all entities that are Capitals.
            // We need 'TileData' to get the starting (x, y) coordinates.
            auto qCapitals = world.query_builder<const Province, const TileData>()
                .with<CapitalOf>(flecs::Wildcard)
                .build();

            // 2. Process each capital individually
            qCapitals.each([&](flecs::entity capitalEntity, const Province&, const TileData& capTile) {

                // Identify the specific Title (Realm) this province belongs to
                flecs::entity realmTitle = capitalEntity.target<CapitalOf>();

                if (!realmTitle.is_valid()) return;

                // --- Dijkstra Algorithm Initialization ---

                int width = tileMap.width;
                int height = tileMap.height;

                // Priority Queue: <Distance, X, Y> (Min-Heap)
                using Node = std::tuple<float, int, int>;
                std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

                // Local distance buffer to track shortest paths during this flood fill
                // Initialized to infinity
                std::vector<std::vector<float>> minDist(
                    width,
                    std::vector<float>(height, std::numeric_limits<float>::infinity())
                );

                // Start at the Capital
                minDist[capTile.x][capTile.y] = 0.0f;
                pq.push({0.0f, capTile.x, capTile.y});

                // Update the capital's own distance immediately
                auto capital = capitalEntity.ensure<Province>();
                capital.distance_to_capital = 0.f;

                // Cardinal directions (N, S, E, W)
                const int dx[] = {0, 0, 1, -1};
                const int dy[] = {1, -1, 0, 0};

                // --- Expansion Loop ---

                while (!pq.empty()) {
                    auto [currentDist, cx, cy] = pq.top();
                    pq.pop();

                    // Optimization: If we found a shorter path to this node already, skip
                    if (currentDist > minDist[cx][cy]) continue;

                    // Check all 4 neighbors
                    for (int i = 0; i < 4; ++i) {
                        int nx = cx + dx[i];
                        int ny = cy + dy[i];

                        // Bounds check
                        if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;

                        flecs::entity neighborEntity = tileMap.tiles[nx][ny];
                        if (!neighborEntity.is_valid()) continue;

                        // Constraint 1: The neighbor must belong to the SAME Realm (Title)
                        if (!neighborEntity.has<InRealm>(realmTitle)) continue;

                        // Constraint 2: Retrieve neighbor data for movement cost calculation
                        Province neighborProv = neighborEntity.ensure<Province>();

                        // Calculate new distance (using the neighbor's movement cost to enter)
                        float newDist = currentDist + neighborProv.movement_cost;

                        if (newDist < minDist[nx][ny]) {
                            minDist[nx][ny] = newDist;
                            pq.push({newDist, nx, ny});

                            // Apply the calculated distance to the component
                            neighborProv.distance_to_capital = newDist;
                        }
                    }
                }
            });
        });
}

void UpdateStats(const flecs::world& ecs, const GameTickSources &timers) {
    auto qPlayerProvinces = ecs.query_builder<Province>()
        .with<InRealm>("$title")
        .with<RulerOf>("$title").src("$player")
        .with<Player>().src("$player")
        .build();

    ecs.system("EstateEffects")
        .kind(flecs::PreUpdate)
        .tick_source(timers.mWeekTimer)
        .run([=](flecs::iter& it) {
            const auto estates = it.world().get<EstatePowers>();

            // Commoners Power

            int commoners = estates.mCommonersPower;
            int change = 0;
            int chance = 0;

            if (commoners > ESTATE_EFFECT_THRESHOLD) {
                change = 1;
                chance = commoners - ESTATE_EFFECT_THRESHOLD;
            } else if (commoners < -ESTATE_EFFECT_THRESHOLD) {
                change = -1;
                chance = std::abs(commoners) - ESTATE_EFFECT_THRESHOLD;
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

            auto player = ecs.entity<Player>();
            auto rulerCulture = player.get<CharacterCulture>().culture;
            auto rulerTraits = GetCulturalTraits(rulerCulture);

            qPlayerProvinces.each([&](flecs::entity t, Province& p) {

                if (Random::GetIntRange(0,100) <= chance+5)
                    p.development =
                        std::clamp<unsigned int>(
                            change + p.development
                            , 0, 100
                        );

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
                    +  10 * (p.terrain != Plains)
                    +  10 * (p.terrain == Mountains)
                    +  10 * (p.roads_level == 0 && (p.biome == Forests || p.biome == Jungles))
                    -  5 * p.roads_level;
            });
        });
}

ProvinceUpdates::ProvinceUpdates(flecs::world &ecs) {
    const auto &timers = ecs.get<GameTickSources>();

    GatherProvinceRevenue(ecs, timers);
    UpdateDistanceToCapital(ecs, timers);
    UpdateStats(ecs, timers);
}
