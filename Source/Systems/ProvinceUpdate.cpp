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
        });
}

void UpdateDistanceToCapital(const flecs::world& ecs, const GameTickSources &timers) {
    // Query all Titles (Kingdoms, Duchies, etc.)
    const auto qTitles = ecs.query<const Title>();

    ecs.system<const TileMap>("ReCalculateDistancesToCapital")
        .kind(flecs::PreUpdate)
        .tick_source(timers.mMonthTimer)
        .each([=](flecs::iter& it, size_t, const TileMap &tileMap) {

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
                    +  15 * (p.terrain != Plains)
                    +  15 * (p.terrain == Mountains)
                    +  15 * (p.roads_level == 0 && (p.biome == Forests || p.biome == Jungles))
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
