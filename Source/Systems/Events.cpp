#include "Events.h"

#include "imgui.h"

EventsSampleScene::EventsSampleScene(const flecs::world& ecs)
{
    void(ecs.component<GameTime>()
        .add(flecs::Singleton)
        .emplace<GameTime>(GameTime{ .mDay = 0 }));

    ecs.system<const EventSchedule, const GameTime>()
        .order_by<EventSchedule>([](
            flecs::entity_t, const EventSchedule *a,
            flecs::entity_t, const EventSchedule *b)
        {
            return (*b < *a) - (*a < *b);
        })
        .each([](flecs::iter &it, size_t i, const EventSchedule &schedule, const GameTime &gameTime)
        {
            if (schedule.mDay > gameTime.mDay)
            {
                // stop iteration?
                return;
            }
            void(it.entity(i).add<FiredEvent>());
        });

    ecs.system<const SamplePopup, const FiredEvent>()
        .each([](const flecs::entity &e, const SamplePopup& p, const FiredEvent&)
        {
            auto name = "Popup##" + std::to_string(e.id());
            if (ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoDocking))
            {
                // ImGui::PushID(e.id());
                ImGui::Text("ID %lu", e.id());
                ImGui::Text("%s", p.mMessage.c_str());
                if (ImGui::Button("Close me"))
                {
                    e.destruct();
                }
                // ImGui::PopID();
            }
            ImGui::End();
        });

    ecs.system<GameTime>()
        .each([](GameTime &t)
        {
            if (ImGui::Begin("Time Controls"))
            {
                ImGui::Text("Current day: %lu", t.mDay);
                if (ImGui::Button("Advance"))
                {
                    t.mDay += 1;
                }
            }
            ImGui::End();
        });

    void(ecs.entity()
        .emplace<EventSchedule>(EventSchedule{ .mDay = 5 })
        .emplace<SamplePopup>(SamplePopup{ "I popped up at day 5" }));

    void(ecs.entity()
        .emplace<EventSchedule>(EventSchedule{ .mDay = 10 })
        .emplace<SamplePopup>(SamplePopup{ "I popped up at day 10" }));
}
