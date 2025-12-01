#include "ProvinceRevenue.hpp"

#include "Characters.hpp"
#include "GameTime.hpp"
#include "Components/Province.hpp"

void DoProvinceRevenueSystems(const flecs::world& ecs, const GameTickSources &timers)
{
    auto qProvinceRuler = ecs.query_builder<Character>("qProvinceRuler")
        .with<RuledBy>("$this").src("$title")
        .with<InRealm>("$title").src("$province")
        .build();

    ecs.system<Province, const GameTime>()
        .tick_source(timers.mMonthTimer)
        .each([=](flecs::iter &it, size_t i, Province &province, const GameTime &gameTime)
        {
            size_t days = gameTime.CountDayChanges();
            qProvinceRuler
                .set_var("province", it.entity(i))
                .each([&](Character &character)
                {
                    character.mMoney += province.income * days;
                });

            // TODO: pay taxes to higher lieges
        });
}
