#include <cmath>
#include <imgui.h>

#include "Events.hpp"
#include "Characters.hpp"
#include "Game.hpp"

// TODO: remove this
#include <SDL3/SDL.h>

#include "Components/Province.hpp"

void CenterNextImGuiWindow()
{
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
}

void DoCharacterBirthSystems(const flecs::world& ecs, const flecs::timer &tickTimer)
{
    // Starts pregnancy events for married couples
    ecs.observer<const Character, const GameTime>()
        .with(ecs.component<MarriedTo>(), flecs::Wildcard)
        .with(Gender::Female)
        .event(flecs::Monitor)
        .each([](flecs::iter &it, size_t i, const Character &c, const GameTime &gameTime)
        {
            flecs::entity characterEntity = it.entity(i);
            const auto &ecs = it.world();
            if (it.event() == flecs::OnAdd)
            {
                flecs::entity spouseEntity = characterEntity.target(ecs.component<MarriedTo>());
                void(characterEntity.child()
                    .add<FiredEvent>()
                    .set<PregnancySaga>({
                        .father = spouseEntity,
                        .mother = characterEntity,
                        .dynasty = characterEntity.target<DynastyMember>(),
                    }));
            } else if (it.event() == flecs::OnRemove)
            {
                characterEntity.children([](flecs::entity child)
                {
                    if (child.has<PregnancySaga>()) return child.destruct();
                });
            }
        });

    ecs.system<PregnancySaga, const GameTime>()
        .with<FiredEvent>()
        .tick_source(tickTimer)
        .each([](flecs::iter &it, size_t i, PregnancySaga &saga, const GameTime &gameTime)
        {
            const auto &entity = it.entity(i);
            auto *father = saga.father.try_get<Character>();
            auto *mother = saga.mother.try_get<Character>();
            if (father == nullptr || mother == nullptr) return entity.destruct();

            switch (saga.stage)
            {
            case PregnancySaga::Attempt:
                {
                    // TODO: make random deterministic
                    if (Random::GetFloat() < 0.2f)
                    {
                        saga.stage = PregnancySaga::Announce;
                    }
                    void(entity.remove<PausesGame>()
                        .remove<FiredEvent>()
                        .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30)));
                }
                break;
            case PregnancySaga::Announce:
                {
                    void(entity.add<PausesGame>());
                    auto name = "Evento - Gravidez##" + std::to_string(entity.id());
                    CenterNextImGuiWindow();
                    if (ImGui::Begin(name.data()))
                    {
                        ImGui::Text("%s e %s vão ter uma criança.", father->mName.data(), mother->mName.data());
                        if (ImGui::Button("Close"))
                        {
                            saga.stage = PregnancySaga::Birth;
                            // TODO: make random deterministic
                            int variation = Random::GetIntRange(30, 60);
                            void(entity.remove<PausesGame>()
                                .remove<FiredEvent>()
                                .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30 * 7 + variation)));
                        }
                    }
                    ImGui::End();
                    break;
                }
            case PregnancySaga::Birth:
                {
                    saga.child = BirthChildCharacter(it.world(), *father, *mother, saga.dynasty);
                    saga.stage = PregnancySaga::BirthAnnounce;
                }
            case PregnancySaga::BirthAnnounce:
                {
                    void(entity.add<PausesGame>());
                    auto name = "Evento - Nascimento##" + std::to_string(entity.id());
                    const auto *child = saga.child.try_get<Character>();
                    CenterNextImGuiWindow();
                    if (ImGui::Begin(name.data()))
                    {
                       ImGui::Text("O filho de %s e %s nasceu: %s", father->mName.data(), mother->mName.data(), child ? child->mName.data() : nullptr);
                       if (ImGui::Button("Close"))
                       {
                           void(entity.destruct());
                       }
                    }
                    ImGui::End();
                    break;
                }
            }
        });
}

void DoAdultMarriageSystems(const flecs::world& ecs, flecs::timer)
{
    // Adds marriage planning to unmarried adults
    ecs.observer<const Character, const GameTime>()
        .with<AgeClass>(AgeClass::Adult)
        .without(ecs.component<MarriedTo>(), flecs::Wildcard)
        .event(flecs::Monitor)
        .each([](flecs::iter &it, size_t i, const Character &c,  const GameTime &gameTime)
        {
            flecs::entity characterEntity = it.entity(i);
            if (it.event() == flecs::OnRemove)
            {
                characterEntity.children([](flecs::entity child)
                {
                    if (child.has<PlanMarriageSaga>()) return child.destruct();
                });
            } else if (it.event() == flecs::OnAdd)
            {
                void(characterEntity.child()
                    .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30))
                    .set<PlanMarriageSaga>({ characterEntity }));
            }
        });

    // TODO: implement the behavior
}

void DoEventSchedulingSystems(const flecs::world& ecs, flecs::timer tickTimer)
{
    void(ecs.component<FiredEvent>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    // Adds a fired tag to events which are scheduled to the past
    ecs.system<const EventSchedule, const GameTime>()
        .kind(flecs::PreUpdate)
        .tick_source(tickTimer)
        .order_by<EventSchedule>([](
            flecs::entity_t, const EventSchedule *a,
            flecs::entity_t, const EventSchedule *b)
        {
            return (*b < *a) - (*a < *b);
        })
        .run([](flecs::iter &it)
        {
            while (it.next())
            {
                auto &gameTime = it.field_at<const GameTime>(1, 0);
                for (const size_t i : it)
                {
                    const auto &schedule = it.field_at<const EventSchedule>(0, i);
                    if (schedule.mTimeSecs > gameTime.mTimeSecs)
                        return it.fini();
                    void(it.entity(i).add<FiredEvent>());
                }
            }
        });
}

void DoGameTimeSystems(const flecs::world& ecs, flecs::timer tickTimer)
{
    void(ecs.component<GameTime>()
        .add(flecs::Singleton));

    // Advances the game time by the set speed
    ecs.system<GameTime, const GameTimers>()
        .write<flecs::TickSource>()
        .kind(flecs::PreUpdate)
        .tick_source(tickTimer)
        .each([](const flecs::iter &it, size_t, GameTime &gameTime, const GameTimers &timers)
        {
            if (it.world().count<PausesGame>() > 0) return;

            float speedChange = it.delta_time() * gameTime.mSpeedAccel;
            if (gameTime.mSpeed < -speedChange)
                gameTime.mSpeed = 0;
            else
                gameTime.mSpeed += speedChange;
            gameTime.mSpeedAccel *= powf(0.1, it.delta_time());

            gameTime.mLastTimeSecs = gameTime.mTimeSecs;
            gameTime.mTimeSecs += uint64_t(it.delta_time() * gameTime.mSpeed * 86400.0);
            if (gameTime.CountDayChanges() != 0)
            {
                // Immediately tick the systems which use DayTimer
                auto &tickSource = timers.mDayTimer.get_mut<EcsTickSource>();
                tickSource.tick = true;
            }
        });

    // Controls for game speed
    ecs.system<GameTime>()
        .kind(flecs::OnUpdate)
        .tick_source(tickTimer)
        .each([](GameTime &t)
        {
            if (ImGui::Begin("Time Controls"))
            {
                ImGui::Text("Day %zu %02zu:%02zu", t.TimeDays(), t.TimeHours(), t.TimeMinutes());
                ImGui::SliderFloat("Speed", &t.mSpeed, 0.0f, 2.0f, "%.2f d/s");
                ImGui::SliderFloat("Accel", &t.mSpeedAccel, -0.5f, 0.5f, "%.2f d/s²");
            }
            ImGui::End();
        });
}

void DoSamplePopupSystem(const flecs::world& ecs, flecs::timer tickTimer)
{
    // Sample Popup for testing event schedules
    ecs.system<const SamplePopup, const FiredEvent>()
        .kind(flecs::OnUpdate)
        .tick_source(tickTimer)
        .each([](const flecs::entity &e, const SamplePopup& p, const FiredEvent&)
        {
            void(e.add<PausesGame>());

            auto name = "Popup##" + std::to_string(e.id());

            CenterNextImGuiWindow();
            if (ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            {
                ImGui::Text("ID %zu", e.id());
                ImGui::Text("%s", p.mMessage.c_str());
                if (ImGui::Button("Close me"))
                {
                    e.destruct();
                }
            }
            ImGui::End();
        });
}

void DoProvinceRevenueSystems(const flecs::world& ecs, const GameTimers &timers)
{
    ecs.system<Province, const GameTime>()
        .tick_source(timers.mDayTimer)
        .each([](flecs::iter &it, size_t i, Province &province, const GameTime &gameTime)
        {
            size_t days = gameTime.CountDayChanges();
            // TODO: see if there is a cleaner way to reference the ruler
            flecs::entity entity = it.entity(i);
            flecs::entity title = entity.target<InRealm>();
            flecs::entity ruler = title.target<RuledBy>();

            auto &character = ruler.get_mut<Character>();
            character.mMoney += province.income * days;

            // TODO: pay taxes to higher lieges
        });
}

void DoCharacterAgingSystem(const flecs::world& ecs, const GameTimers& timers)
{
    ecs.system<Character, const GameTime>()
        .tick_source(timers.mDayTimer)
        .each([](flecs::entity e, Character &character, const GameTime &gameTime)
        {
            character.mAgeDays += gameTime.CountDayChanges();
            if (!e.has(AgeClass::Adult) && character.mAgeDays > 16 * 360)
                void(e.add(AgeClass::Adult));
        });
}

EventsSampleScene::EventsSampleScene(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTimers>();

    DoGameTimeSystems(ecs, timers.mTickTimer);

    DoEventSchedulingSystems(ecs, timers.mTickTimer);

    DoAdultMarriageSystems(ecs, timers.mTickTimer);

    DoCharacterBirthSystems(ecs, timers.mTickTimer);

    DoSamplePopupSystem(ecs, timers.mTickTimer);

    DoProvinceRevenueSystems(ecs, timers);

    DoCharacterAgingSystem(ecs, timers);

    EventsSampleScene::InitializeEntities(ecs);
}

void EventsSampleScene::InitializeEntities(const flecs::world& ecs)
{
    void(ecs.component<GameTime>().set<GameTime>({}));

    void(ecs.entity()
        .emplace<SamplePopup>(SamplePopup{ "Bem vindo ao Era dos Fidalgos!" })
        .add<FiredEvent>());

    void(ecs.entity()
        .emplace<EventSchedule>(EventSchedule::AtDay(5))
        .emplace<SamplePopup>(SamplePopup{ "I popped up at day 5" }));

    void(ecs.entity()
        .emplace<EventSchedule>(EventSchedule::AtDay(10))
        .emplace<SamplePopup>(SamplePopup{ "I popped up at day 10" }));
}
