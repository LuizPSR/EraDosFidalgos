#include <cmath>
#include <imgui.h>

#include "Events.hpp"
#include "Characters.hpp"
#include "Game.hpp"

// TODO: remove this
#include <SDL3/SDL.h>

EventsSampleScene::EventsSampleScene(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTimers>();
    const flecs::entity tickTimer = timers.mTickTimer;

    void(ecs.component<FiredEvent>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    void(ecs.component<GameTime>()
        .add(flecs::Singleton)
        .emplace<GameTime>(GameTime{}));

    ecs.system<GameTime, GameTimers>()
        .write<flecs::Timer>()
        .kind(flecs::PreUpdate)
        .tick_source(tickTimer)
        .each([](const flecs::iter &it, size_t, GameTime &gameTime, GameTimers &timers)
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

    ecs.system<const SamplePopup, const FiredEvent>()
        .kind(flecs::OnUpdate)
        .tick_source(tickTimer)
        .each([](const flecs::entity &e, const SamplePopup& p, const FiredEvent&)
        {
            void(e.add<PausesGame>());

            auto name = "Popup##" + std::to_string(e.id());

            const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
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

    ecs.observer<Character, const GameTime>()
        .event(flecs::OnAdd)
        .each([](flecs::iter &it, size_t i, const Character &c,  const GameTime& gameTime)
        {
            SDL_Log("Observer fired");
            flecs::entity characterEntity = it.entity(i);
            const auto &ecs = it.world();
            if (!characterEntity.has_relation(ecs.component<MarriedTo>()))
            {
                SDL_Log("Planning marriage for %s", c.mName.c_str());
                ecs.entity()
                    .set<EventSchedule>(EventSchedule::InXDays(gameTime, 30))
                    .set<PlanMarriage>({ characterEntity });
            }
        });

    // TOOD: finish this
    ecs.system<PregnancySaga, const GameTime>()
        .with<FiredEvent>()
        .each([](flecs::iter &it, size_t i, PregnancySaga &saga, const GameTime &gameTime)
        {
            switch (saga.stage)
            {
            case PregnancySaga::Stage::Attempt:
                break;
            case PregnancySaga::Stage::Announce:
                break;
            case PregnancySaga::Stage::Birth:
                break;
            }
        });

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
