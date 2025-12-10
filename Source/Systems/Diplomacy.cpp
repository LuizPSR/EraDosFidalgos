#include <flecs.h>
#include <toml++/toml.hpp>
#include <filesystem>

#include "Diplomacy.hpp"
#include "Characters.hpp"
#include "Events.hpp"
#include "Game.hpp"
#include "imgui.h"
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

    const auto path = std::filesystem::path(SDL_GetBasePath()) / "Assets" / "DiploEvents.toml";
    static toml::table eventsTbl = toml::parse_file(path.string());

    void(ecs.system<const Title, const Title>("DiploEventSpawner")
        .term_at(0).src("$realm")
        .term_at(1).src("$neighbor")
        .with<Player>()
        .with<RulerOf>("$realm")
        .with<Neighboring>("$neighbor").src("$realm")
        .tick_source(timers.mMonthTimer)
        .each([=](flecs::iter &it, size_t, const Title &a, const Title &b)
        {
            if (Random::GetFloat() >= 0.2f) return;
            const auto *eventsArray = eventsTbl["event"].as_array();
            size_t idx = Random::GetIntRange(0, eventsArray->size() - 1);
            const auto &tbl = *eventsArray->get_as<toml::table>(idx);
            std::vector<DiploEventChoice> choices;
            tbl["option"].as_array()->for_each([&](const toml::table &t)
            {
                choices.push_back((DiploEventChoice){
                    .mText = t["text"].as_string()->get(),
                    .mRelationChange = (int8_t)(t["relation_change"].as_integer()->get()),
                });
            });
            it.world().entity()
                .add<PausesGame>()
                .set<DiploEvent>({
                    .mTitle = tbl["title"].as_string()->get(),
                    .mMessage = tbl["message"].as_string()->get(),
                    .mChoices = choices,
                    .mSourceRealm = it.get_var("realm"),
                    .mTargetRealm = it.get_var("neighbor")
                });
        }));

    ecs.system<const DiploEvent>("RenderDiploEvents")
        .each([](flecs::entity entity, const DiploEvent &event)
        {
            std::string title = event.mTitle + "##" + std::to_string(entity.id());
            ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Appearing);
            if (ImGui::Begin(title.data()))
            {
                ImGui::TextWrapped("%s", event.mMessage.data());
                for (const auto &choice: event.mChoices)
                {
                    if (ImGui::Button(choice.mText.data()))
                    {
                        auto &relation = event.mSourceRealm.ensure<RealmRelation>(event.mTargetRealm);
                        relation.relations += std::clamp((int)relation.relations + choice.mRelationChange, -128, 127);;
                        entity.destruct();
                    }
                    if (ImGui::BeginItemTooltip())
                    {
                        ImGui::Text("Relações: %d", choice.mRelationChange);
                        ImGui::EndTooltip();
                    }
                }
            }
            ImGui::End();
        });
}
