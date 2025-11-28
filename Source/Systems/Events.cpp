#include <cmath>
#include <imgui.h>

#include "Events.hpp"

EventsSampleScene::EventsSampleScene(const flecs::world& ecs)
{
    void(ecs.component<FiredEvent>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    void(ecs.component<GameTime>()
        .add(flecs::Singleton)
        .emplace<GameTime>(GameTime{}));

    ecs.system<GameTime>()
        .each([](const flecs::iter &it, size_t, GameTime &gameTime)
        {
            float speedChange = it.delta_time() * gameTime.mSpeedAccel;
            if (gameTime.mSpeed < -speedChange)
                gameTime.mSpeed = 0;
            else
                gameTime.mSpeed += speedChange;
            gameTime.mSpeedAccel *= powf(0.1, it.delta_time());
            gameTime.mTime += it.delta_time() * gameTime.mSpeed;
        });

    ecs.system<const EventSchedule, const GameTime>()
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
                    if (schedule.mTime > gameTime.mTime)
                        return it.fini();
                    void(it.entity(i).add<FiredEvent>());
                }
            }
        });

    ecs.system<const SamplePopup, const FiredEvent>()
        .each([](const flecs::entity &e, const SamplePopup& p, const FiredEvent&)
        {
            auto name = "Popup##" + std::to_string(e.id());
            if (ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            {
                ImGui::Text("ID %lu", e.id());
                ImGui::Text("%s", p.mMessage.c_str());
                if (ImGui::Button("Close me"))
                {
                    e.destruct();
                }
            }
            ImGui::End();
        });

    ecs.system<GameTime>()
        .each([](GameTime &t)
        {
            if (ImGui::Begin("Time Controls"))
            {
                ImGui::Text("Day %.0f %02.0f:%02.0f", t.mTime,
                    fmodf(t.mTime, 1.0f) * 24,
                    fmodf(t.mTime, 1.0f / 24) * 24 * 60);
                ImGui::SliderFloat("Speed", &t.mSpeed, 0.0f, 2.0f, "%.2f d/s");
                ImGui::SliderFloat("Accel", &t.mSpeedAccel, -0.5f, 0.5f, "%.2f d/s²");
            }
            ImGui::End();
        });

    void(ecs.entity()
        .emplace<SamplePopup>(SamplePopup{ "Bem vindo ao Era dos Fidalgos!" })
        .add<FiredEvent>());

    void(ecs.entity()
        .emplace<EventSchedule>(EventSchedule{ .mTime = 5 })
        .emplace<SamplePopup>(SamplePopup{ "I popped up at day 5" }));

    void(ecs.entity()
        .emplace<EventSchedule>(EventSchedule{ .mTime = 10 })
        .emplace<SamplePopup>(SamplePopup{ "I popped up at day 10" }));
}
