#include <imgui.h>

#include "GameTime.hpp"
#include "Events.hpp"
#include "Game.hpp"

GameTime::GameTime(const flecs::world& ecs)
{
    mTickTimer = ecs.timer("TickTimer");
    mDayTimer = ecs.timer("DayTimer");
    mDayTimer.stop();
    mWeekTimer = ecs.timer("WeekTimer");
    mWeekTimer.stop();
    mMonthTimer = ecs.timer("MonthTimer");
    mMonthTimer.stop();
}

void DoGameTimeSystems(const flecs::world& ecs, flecs::timer tickTimer)
{
    void(ecs.component<GameTime>()
        .add(flecs::Singleton));

    // Advances the game time by the set speed
    ecs.system<GameTime, const GameTime>()
        .write<flecs::TickSource>()
        .kind(flecs::PreUpdate)
        .tick_source(tickTimer)
        .each([](const flecs::iter &it, size_t, GameTime &gameTime, const GameTime &timers)
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
            if (gameTime.CountWeekChanges() != 0)
            {
                auto &tickSource = timers.mWeekTimer.get_mut<EcsTickSource>();
                tickSource.tick = true;
            }
            if (gameTime.CountMonthChanges() != 0)
            {
                auto &tickSource = timers.mMonthTimer.get_mut<EcsTickSource>();
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
