#include <cmath>
#include <imgui.h>
#include <toml++/toml.hpp>
#include <SDL3/SDL_filesystem.h>
#include <filesystem>

#include "Events.h"

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

// TODO: Reorganize this


struct CharacterBuilder
{
    const flecs::world& ecs;
    flecs::entity rulerPrefab;
    toml::array *dynastyNames;
    toml::array *provinceNames;
    toml::array *maleNames;
    toml::array *femaleNames;

    flecs::entity CreateDynastyWithKingdomAndFamily(size_t index) const;
    flecs::entity CreateCharacterAndFamily(
        const std::string& char_name,
        const flecs::entity& dynasty_entity,
        const flecs::entity& spouse_entity = flecs::entity::null()) const;
};

flecs::entity CharacterBuilder::CreateCharacterAndFamily(
        const std::string& char_name,
        const flecs::entity& dynasty_entity,
        const flecs::entity& spouse_entity) const
{
    flecs::entity character = ecs.entity()
        .is_a(rulerPrefab)
        .set<Character>({char_name})
        .add<DynastyMember>(dynasty_entity);

    if (spouse_entity.is_valid())
    {
        void(character.add<MarriedTo>(spouse_entity));
    }

    return character;
}

flecs::entity CharacterBuilder::CreateDynastyWithKingdomAndFamily(const size_t index) const
{
    auto dName = dynastyNames->get_as<std::string>(index);
    auto dynasty = ecs.entity().set<Dynasty>({ dName->get() });

    auto kingdom = ecs.entity().set<Title>({ dName->get() + " Kingdom" });

    auto pName = provinceNames->get_as<std::string>(index);
    auto capital = ecs.entity()
        .set<Province>({ dName->get() + " " + pName->get() })
        .add<InRealm>(kingdom);

    for (size_t i = 0; i < 9; i += 1)
    {
        auto pName = provinceNames->get_as<std::string>(index + i);
        void(ecs.entity()
            .set<Province>({ dName->get() + " " + pName->get() })
            .add<InRealm>(kingdom));
    }

    auto rulerName = maleNames->get_as<std::string>(index);
    auto ruler = CreateCharacterAndFamily(rulerName->get(), dynasty)
        .add<Ruler>(kingdom)
        .add<DynastyHead>(dynasty);
    auto spouseName = femaleNames->get_as<std::string>(index);
    auto spouse = CreateCharacterAndFamily(spouseName->get(), dynasty)
        .add<MarriedTo>(ruler)
        .add<DynastyMember>(dynasty);

    return dynasty;
}

// --- CharactersModule Implementation ---

CharactersModule::CharactersModule(const flecs::world& ecs)
{
    void(ecs.component<ShowDetails>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    void(ecs.component<Character>()
        .add(flecs::With, ecs.component<ShowDetails>()));

    // Relationship Tags
    void(ecs.component<Ruler>().add(flecs::Acyclic).add(flecs::Symmetric));
    void(ecs.component<Courtier>().add(flecs::Symmetric));
    void(ecs.component<MarriedTo>().add(flecs::Symmetric));
    void(ecs.component<DynastyMember>().add(flecs::Symmetric));
    void(ecs.component<DynastyHead>()
        .add(flecs::With, ecs.component<DynastyMember>())
        .add(flecs::Symmetric));
    void(ecs.component<InRealm>().add(flecs::Transitive)); // Used for hierarchy

    // Prefabs
    flecs::entity rulerPrefab = ecs.prefab()
        .add<Character>();
    void(rulerPrefab.disable<ShowDetails>());

    const auto path = std::filesystem::path(SDL_GetBasePath()) / "Assets" / "Names.toml";
    toml::table tbl = toml::parse_file(path.string());

    auto builder = CharacterBuilder {
        .ecs = ecs,
        .rulerPrefab = rulerPrefab,
        .dynastyNames = tbl["dynasties"]["names"].as_array(),
        .provinceNames = tbl["provinces"]["names"].as_array(),
        .maleNames = tbl["characters"]["male"]["names"].as_array(),
        .femaleNames = tbl["characters"]["female"]["names"].as_array(),
    };
    for (size_t i = 0; i < 3; i += 1)
        void(builder.CreateDynastyWithKingdomAndFamily(i));

    // 3. Initialize Cached Queries

    // --- Cached Queries Declarations ---

    // Query to find all characters who are a Ruler of any Title
    flecs::query<const Character> qRulers = ecs.query_builder<const Character>()
        .with<Ruler>(flecs::Wildcard)
        .build();

    // Query to find all Characters who are members of a specific Dynasty
    flecs::query<const Character> q_dynasty_members;

    // Query to find all Titles/Provinces that belong to a larger Title (e.g., a Kingdom)
    flecs::query<> qInRealm = ecs.query_builder<>()
        .with<InRealm>("$who")
        .build();

    ecs.system<>()
        .run([qRulers, qInRealm](const flecs::iter &it)
        {
            const auto &ecs = it.world();
            RenderCharacterOverviewWindow(ecs, qRulers, qInRealm);
        });
}

// 1. Renders the main window listing all rulers.
void RenderCharacterOverviewWindow(
    const flecs::world& ecs,
    const flecs::query<const Character> &qRulers,
    const flecs::query<> &qInRealm)
{
    if (ImGui::Begin("👑 Character Overview"))
    {
        ImGui::BeginTable("RulerListCols", 3);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Ruler Name");
        ImGui::TableNextColumn(); ImGui::Text("Primary Title");
        ImGui::TableNextColumn(); ImGui::Text("Action");

        // Iterate through all Rulers
        qRulers
            .each([&](flecs::entity ruler, const Character& c)
            {

                // Get the primary Title this character rules over
                // We reuse q_ruled_titles, scoped to the current ruler entity.
                std::string title_name = "No Title";
                auto title = ruler.target_for<Title>(ecs.component<Ruler>());
                if (title.is_valid())
                    title_name = title.get<Title>().name;

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%s", c.name.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%s", title_name.c_str());

                bool showDetails = ruler.enabled<ShowDetails>();
                // Open Detail Button
                ImGui::TableNextColumn();
                std::string button_label = "Details##" + std::to_string(ruler.id());
                if (ImGui::Button(button_label.c_str()))
                {
                    if (showDetails)
                        void(ruler.disable<ShowDetails>());
                    else
                        void(ruler.enable<ShowDetails>());
                }
                if (showDetails)
                    RenderCharacterDetailWindow(ecs, ruler, qInRealm, c);
            });

        ImGui::EndTable();
    }
    ImGui::End();
}

// 2. Renders a detailed window for a single character entity.
void RenderCharacterDetailWindow(
    const flecs::world &ecs,
    const flecs::entity character,
    const flecs::query<>& qInRealm,
    const Character& c)
{
    const std::string window_name = c.name + " - Details";
    bool open = character.enabled<ShowDetails>();

    if (ImGui::Begin(window_name.c_str(), &open))
    {
        // --- BASIC INFO & DYNASTY ---
        flecs::entity dynasty_target = character.target<DynastyMember>();
        auto *d = dynasty_target.try_get<Dynasty>();
        ImGui::Text("Dynasty: %s", d ? d->name.c_str() : "None");

        // --- MARRIAGE ---
        flecs::entity spouse_target = character.target<MarriedTo>();
        if (spouse_target.is_valid())
        {
            auto* spouse = spouse_target.try_get<Character>();
            ImGui::Text("Married To: %s", spouse ? spouse->name.c_str() : "Single");
        }

        // ImGui::Separator();
        ImGui::Text("🏰 Landed Holdings:");

        // --- TITLES & PROVINCES (Hierarchy Traversal) ---
        // Reuse q_ruled_titles to find the direct holdings
        if (auto title_entity = character.target_for<Title>(ecs.component<Ruler>()))
        {
            if (const auto* t = title_entity.try_get<Title>();
                ImGui::TreeNodeEx(t->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Find all Vassal Titles and Provinces owned by this Title
                qInRealm.set_var("who", title_entity)
                    .each([&](flecs::entity holding_entity) {
                        // Check if it's a Province
                        auto p = holding_entity.try_get<Province>();
                        if (p) {
                            ImGui::BulletText("Province: %s (Income: $%.2f)", p->name.c_str(), p->income);
                        }

                        // Check if it's a Vassal Title (Title to Title: MemberOf)
                        if (auto *vassal_t = holding_entity.try_get<Title>()) {
                            ImGui::BulletText("Vassal Title: %s", vassal_t->name.c_str());
                            // Note: You might want to recursively check who rules this vassal title here.
                        }
                    });

                ImGui::TreePop();
            }
        }
    }

    if (!open)
    {
        void(character.disable<ShowDetails>());
    }

    ImGui::End();
}
