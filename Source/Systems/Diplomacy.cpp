#include <flecs.h>

#include "Diplomacy.hpp"

#include "Characters.hpp"
#include "Game.hpp"
#include "Components/Province.hpp"
#include "SDL3/SDL_log.h"

DiplomacyModule::DiplomacyModule(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTickSources>();

    void(ecs.component<RealmRelation>().add(flecs::Symmetric));

    const auto getTopRealms = ecs.query_builder<const Title, const Character>()
        .term_at(1).src("$ruler")
        .without<InRealm>()
        .with<RulerOf>("$this").src("$ruler")
        .build();

    void(ecs.system("diploEvents")
        .tick_source(timers.mMonthTimer)
        .each([=]()
        {
            size_t count = 0;
            getTopRealms.each([&](const Title &title, const Character &character)
            {
                count += 1;
            });
            SDL_Log("Counts: %zd", count);
        }));
}
