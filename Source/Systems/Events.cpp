#include <cmath>
#include <imgui.h>

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

// --- Helper Function to Create Character & Family ---

// This simplifies the creation of a character and their immediate family/dynasty.
flecs::entity create_character_and_family(
    const flecs::world& ecs,
    const flecs::entity& rulerPrefab,
    const std::string& char_name,
    const flecs::entity& dynasty_entity,
    const flecs::entity& spouse_entity = flecs::entity::null())
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

// --- CharactersModule Implementation ---

CharactersModule::CharactersModule(const flecs::world& ecs)
{
    void(ecs.component<ShowDetails>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    void(ecs.component<Character>()
        .add(flecs::With, ecs.component<ShowDetails>()));

    // Relationship Tags
    void(ecs.component<RulerOf>().add(flecs::Acyclic)); // Rulers don't rule themselves
    void(ecs.component<CourtierOf>());
    void(ecs.component<MarriedTo>().add(flecs::Symmetric)); // Marriage is reciprocal
    void(ecs.component<DynastyMember>());
    void(ecs.component<InRealm>().add(flecs::Transitive)); // Used for hierarchy

    // Prefabs
    flecs::entity rulerPrefab = ecs.prefab()
        .add<Character>();
    void(rulerPrefab.disable<ShowDetails>());

    // --- Dynasties ---
    flecs::entity d_stark = ecs.entity("Dynasty_Stark").set<Dynasty>({"Stark"});
    flecs::entity d_lannister = ecs.entity("Dynasty_Lannister").set<Dynasty>({"Lannister"});
    flecs::entity d_baratheon = ecs.entity("Dynasty_Baratheon").set<Dynasty>({"Baratheon"});

    // --- Kingdoms (Top-level Titles) ---
    flecs::entity k_north = ecs.entity("Kingdom_of_the_North").set<Title>({"The North"});
    flecs::entity k_westerlands = ecs.entity("Kingdom_of_the_Westerlands").set<Title>({"The Westerlands"});
    flecs::entity k_stormlands = ecs.entity("Kingdom_of_the_Stormlands").set<Title>({"The Stormlands"});

    // --- CHARACTERS AND THEIR HOLDINGS ---

    // =========================================================================
    // KINGDOM OF THE NORTH (Stark)
    // =========================================================================

    // ** KING/RULER **
    flecs::entity c_ned_spouse = create_character_and_family(ecs, rulerPrefab, "Catelyn Tully", d_stark);
    flecs::entity c_ned = create_character_and_family(ecs, rulerPrefab, "Eddard Stark", d_stark, c_ned_spouse);
    void(c_ned.add<RulerOf>(k_north));

    // ** VASSALS (Dukes/Counts/Lords) **

    // 1. Lord of White Harbor (House Manderly)
    flecs::entity t_white_harbor = ecs.entity("Title_White_Harbor").set<Title>({"White Harbor"});
    void(t_white_harbor.add<InRealm>(k_north)); // Vassal Title belongs to Kingdom Title
    flecs::entity c_wyllis = create_character_and_family(ecs, rulerPrefab, "Wyllis Manderly", d_stark);
    void(c_wyllis.add<RulerOf>(t_white_harbor));

    // Provinces for White Harbor
    void(ecs.entity().set<Province>({"Harbor Farms", 100.0f}).add<InRealm>(t_white_harbor));
    void(ecs.entity().set<Province>({"The Port", 350.5f}).add<InRealm>(t_white_harbor));
    void(ecs.entity().set<Province>({"Swamp Lands", 50.0f}).add<InRealm>(t_white_harbor));

    // 2. Lord of Winterfell (House Bolton - *placeholder for sample*)
    flecs::entity t_winterfell = ecs.entity("Title_Winterfell").set<Title>({"Winterfell"});
    void(t_winterfell.add<InRealm>(k_north));
    flecs::entity c_roose = create_character_and_family(ecs, rulerPrefab, "Roose Bolton", d_stark); // Using Stark as a placeholder dynasty
    void(c_roose.add<RulerOf>(t_winterfell));

    // Provinces for Winterfell
    void(ecs.entity().set<Province>({"North Forest", 80.0f}).add<InRealm>(t_winterfell));
    void(ecs.entity().set<Province>({"Iron Mines", 220.0f}).add<InRealm>(t_winterfell));
    void(ecs.entity().set<Province>({"River Lands", 130.0f}).add<InRealm>(t_winterfell));

    // 3. Lord of Karhold (House Karstark)
    flecs::entity t_karhold = ecs.entity("Title_Karhold").set<Title>({"Karhold"});
    void(t_karhold.add<InRealm>(k_north));
    flecs::entity c_rickard = create_character_and_family(ecs, rulerPrefab, "Rickard Karstark", d_stark);
    void(c_rickard.add<RulerOf>(t_karhold));

    // Provinces for Karhold
    void(ecs.entity().set<Province>({"Mountain Pass", 60.0f}).add<InRealm>(t_karhold));
    void(ecs.entity().set<Province>({"East Coast", 190.0f}).add<InRealm>(t_karhold));
    void(ecs.entity().set<Province>({"Fjord Fishery", 110.0f}).add<InRealm>(t_karhold));

    // =========================================================================
    // KINGDOM OF THE WESTERLANDS (Lannister)
    // =========================================================================

    // ** KING/RULER **
    flecs::entity c_tywin_spouse = create_character_and_family(ecs, rulerPrefab, "Joanna Lannister", d_lannister);
    flecs::entity c_tywin = create_character_and_family(ecs, rulerPrefab, "Tywin Lannister", d_lannister, c_tywin_spouse);
    void(c_tywin.add<RulerOf>(k_westerlands));

    // ... (Add 3 vassals, 9 provinces, and families for Westerlands and Stormlands similarly) ...
    // Note: Due to space, the remaining samples are abbreviated, but follow the same pattern.

    // 3. Initialize Cached Queries

    // --- Cached Queries Declarations ---

    // Query to find all characters who are a Ruler of any Title
    flecs::query<const Character> qRulers = ecs.query_builder<const Character>()
        .with<RulerOf>(flecs::Wildcard)
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
                auto title = ruler.target_for<Title>(ecs.component<RulerOf>());
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
        if (auto title_entity = character.target_for<Title>(ecs.component<RulerOf>()))
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
