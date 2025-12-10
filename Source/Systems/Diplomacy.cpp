#include <flecs.h>

#include "Diplomacy.hpp"

#include "Characters.hpp"
#include "Game.hpp"
#include "MapGenerator.hpp"
#include "Random.hpp"
#include "Components/Province.hpp"

DiplomacyModule::DiplomacyModule(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTickSources>();

    void(ecs.component<RealmRelation>().add(flecs::Symmetric));
    void(ecs.component<Neighboring>().add(flecs::Symmetric));

    const auto topmostRealm = ecs.query_builder<const Title>("GetTopmostRealm")
        .with<InRealm>("$this").src("$province")
        .without<InRealm>(flecs::Wildcard)
        .build();

    void(ecs.system<const TileMap>("UpdateNeighbors")
        .write<Neighboring>()
        .tick_source(timers.mWeekTimer)
        .multi_threaded()
        .each([=](const flecs::iter &it, size_t, const TileMap &tileMap)
        {
            const auto &ecs = it.world();
            ecs.remove_all<Neighboring>();
            for (int i = 0; i + 1 < tileMap.width; i += 1)
            for (int j = 0; j + 1 < tileMap.height; j += 1)
            {
                const auto r0 = topmostRealm.set_var("province", tileMap.tiles[i][j]).first();
                if (!r0.is_valid()) continue;
                const auto r1 = topmostRealm.set_var("province", tileMap.tiles[i + 1][j]).first();
                const auto r2 = topmostRealm.set_var("province", tileMap.tiles[i + 1][j + 1]).first();
                const auto r3 = topmostRealm.set_var("province", tileMap.tiles[i][j + 1]).first();
                if (r1.is_valid() && r1 != r0) void(r0.add<Neighboring>(r1));
                if (r2.is_valid() && r2 != r0) void(r0.add<Neighboring>(r2));
                if (r3.is_valid() && r3 != r0) void(r0.add<Neighboring>(r3));
            }
        }));

    void(ecs.system<const Title, const Title>("DiploEvents")
        .term_at(0).src("$realm")
        .term_at(1).src("$neighbor")
        .with<Player>()
        .with<RulerOf>("$realm")
        .with<Neighboring>("$neighbor").src("$realm")
        .tick_source(timers.mMonthTimer)
        .each([=](const Title &a, const Title &b)
        {
            if (Random::GetFloat() >= 0.2f) return;
        }));
}
