#include <imgui.h>
#include <flecs.h>
#include <toml++/toml.hpp>
#include <SDL3/SDL_filesystem.h>
#include <filesystem>

#include "Characters.hpp"

#include "Game.hpp"
#include "../Components/Province.hpp"

#include "Components/Dynasty.hpp"
// TODO: make random deterministic
#include "GameTime.hpp"
#include "Random.hpp"

struct CharacterBuilder
{
    const flecs::world ecs;
    toml::node_view<toml::node> dynastyNames;
    toml::node_view<toml::node> provinceNames;
    toml::node_view<toml::node> maleNames;
    toml::node_view<toml::node> femaleNames;

    std::string GenProvinceName() const;
    std::string GenDynastyName() const;
    std::string GenMaleName() const;
    std::string GenFemaleName() const;

    flecs::entity CreateDynastyWithKingdomAndFamily(bool isPlayer = false) const;
    flecs::entity CreateCharacter(
        const std::string& char_name,
        const flecs::entity& dynasty_entity,
        bool isMale,
        flecs::entity baseEntity) const;
    flecs::entity CreateCharacter(
        const std::string& char_name,
        const flecs::entity& dynasty_entity,
        bool isMale) const
    {
        return CreateCharacter(char_name, dynasty_entity, isMale, ecs.entity());
    }
};

flecs::entity CharacterBuilder::CreateCharacter(
    const std::string& char_name,
    const flecs::entity& dynasty_entity,
    bool isMale,
    flecs::entity baseEntity) const
{
    flecs::entity character = baseEntity
        .set<Character>({
            .mName = char_name,
            .mMoney = 123,
            .mAgeDays = 20 * 360,
        });

    if (dynasty_entity.is_valid())
        void(character.add<DynastyMember>(dynasty_entity));

    void(character.disable<ShowCharacterDetails>());
    if (isMale)
    {
        void(character.add(Gender::Male));
    } else
    {
        void(character.add(Gender::Female));
    }

    return character;
}

// TODO: cleanup the code below, because it's ugly

std::string getRandomArrayElement(const toml::array *array)
{
    size_t index = Random::GetIntRange(0, array->size() - 1);
    return std::string(*array->get_as<std::string>(index));
}

std::string CharacterBuilder::GenProvinceName() const
{
    return getRandomArrayElement(provinceNames["prefixes"].as_array())
        + getRandomArrayElement(provinceNames["suffixes"].as_array());
}

std::string CharacterBuilder::GenDynastyName() const
{
    return getRandomArrayElement(dynastyNames["prefixes"].as_array())
        + getRandomArrayElement(dynastyNames["suffixes"].as_array());
}

std::string CharacterBuilder::GenMaleName() const
{
    return getRandomArrayElement(maleNames["prefixes"].as_array())
        + getRandomArrayElement(maleNames["infixes"].as_array())
        + getRandomArrayElement(maleNames["suffixes"].as_array());
}

std::string CharacterBuilder::GenFemaleName() const
{
    return getRandomArrayElement(femaleNames["prefixes"].as_array())
        + getRandomArrayElement(femaleNames["infixes"].as_array())
        + getRandomArrayElement(femaleNames["suffixes"].as_array());
}

flecs::entity CharacterBuilder::CreateDynastyWithKingdomAndFamily(bool isPlayer) const
{
    auto dName = GenDynastyName();

    auto dynasty = ecs.entity().set<Dynasty>({ dName });

    auto kingdom = ecs.entity().set<Title>({ dName + " Kingdom" });

    auto capital = ecs.entity()
        .set<Province>({ dName + " " + GenProvinceName() })
        .add<InRealm>(kingdom);

    for (size_t i = 0; i < 9; i += 1)
    {
        void(ecs.entity()
            .set<Province>({ dName + " " + GenProvinceName() })
            .add<InRealm>(kingdom));
    }

    auto rulerName = GenMaleName();
    auto ruler = CreateCharacter(rulerName, dynasty, true, isPlayer ? ecs.entity<Player>() : ecs.entity())
        .add<DynastyHead>(dynasty);
    void(kingdom.add<RuledBy>(ruler));

    auto spouseName = GenFemaleName();
    auto spouse = CreateCharacter(spouseName, dynasty, false)
        .add<MarriedTo>(ruler)
        .add<DynastyMember>(dynasty);
    void(ruler.add<MarriedTo>(spouse));

    return dynasty;
}

void DoCreateCharacterBuilder(const flecs::world& ecs)
{
    void(ecs.component<CharacterBuilder>()
        .add(flecs::Singleton));

    const auto path = std::filesystem::path(SDL_GetBasePath()) / "Assets" / "Names.toml";
    static toml::table tbl = toml::parse_file(path.string());

    ecs.component<CharacterBuilder>()
        .emplace<CharacterBuilder>(CharacterBuilder {
            .ecs = ecs.get_world(),
            .dynastyNames = tbl["dynasties"]["names"],
            .provinceNames = tbl["provinces"]["names"],
            .maleNames = tbl["characters"]["male"]["names"],
            .femaleNames = tbl["characters"]["female"]["names"],
        });
}

CharactersModule::CharactersModule(const flecs::world& ecs)
{
    const flecs::entity tickTimer = ecs.get<GameTime>().mTickTimer;

    DoCreateCharacterBuilder(ecs);

    void(ecs.component<ShowCharacterDetails>()
        .add(flecs::Trait)
        .add(flecs::CanToggle));

    void(ecs.component<Character>()
        .add(flecs::With, ecs.component<ShowCharacterDetails>()));

    // Relationship Tags
    void(ecs.component<RuledBy>().add(flecs::Exclusive).add(flecs::Acyclic).add(flecs::Transitive));
    void(ecs.component<MarriedTo>().add(flecs::Exclusive).add(flecs::Symmetric));
    void(ecs.component<Courtier>().add(flecs::Exclusive));
    void(ecs.component<DynastyMember>().add(flecs::Exclusive));
    void(ecs.component<DynastyHead>()
        .add(flecs::With, ecs.component<DynastyMember>())
        .add(flecs::Exclusive)
        .add(flecs::Symmetric));
    void(ecs.component<InRealm>().add(flecs::Exclusive).add(flecs::Transitive));

    // 3. Initialize Cached Queries

    // --- Cached Queries Declarations ---

    CharacterQueries queries = {
        .qAllRulers = ecs.query_builder<const Character,  const Title>("qAllRulers")
            .term_at(1).src("$title")
            .with<RuledBy>("$this").src("$title")
            .build(),
        .qProvincesOfTitle = ecs.query_builder<const Province, const Title>("qProvincesOfTitle")
            .term_at(1).src("$title")
            .with<InRealm>("$title").src("$this")
            .build(),
    };

    ecs.system<>()
        .tick_source(tickTimer)
        .run([queries](const flecs::iter &it)
        {
            const auto &ecs = it.world();
            RenderCharacterOverviewWindow(ecs, queries);
        });
}

void CreateKingdoms(const flecs::world &ecs, size_t count)
{
    const auto &builder = ecs.get<CharacterBuilder>();
    if (!ecs.entity<Player>().has<Character>())
        void(builder.CreateDynastyWithKingdomAndFamily(true));
    for (size_t i = 0; i < count; i += 1)
        void(builder.CreateDynastyWithKingdomAndFamily());
}

// 1. Renders the main window listing all rulers.
void RenderCharacterOverviewWindow(const flecs::world& ecs, const CharacterQueries& queries)
{
    if (ImGui::Begin("Character Overview"))
    {
        ImGui::BeginTable("RulerListCols", 3);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Ruler Name");
        ImGui::TableNextColumn(); ImGui::Text("Primary Title");
        ImGui::TableNextColumn(); ImGui::Text("Action");

        // Iterate through all Rulers
        queries.qAllRulers
            .each([&](flecs::iter it, size_t i, const Character &c, const Title &title)
            {
                flecs::entity ruler = it.entity(i), titleEntity = it.get_var("title");

                // Get the primary Title this character rules over
                // We reuse q_ruled_titles, scoped to the current ruler entity.
                std::string title_name = title.name;

                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%s", c.mName.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%s", title_name.c_str());

                bool showDetails = ruler.enabled<ShowCharacterDetails>();
                // Open Detail Button
                ImGui::TableNextColumn();
                std::string button_label = "Details##" + std::to_string(ruler.id());
                if (ImGui::Button(button_label.c_str()))
                {
                    if (showDetails)
                        void(ruler.disable<ShowCharacterDetails>());
                    else
                        void(ruler.enable<ShowCharacterDetails>());
                }
                if (showDetails)
                    RenderCharacterDetailWindow(ecs, ruler, titleEntity, c, queries);
            });

        ImGui::EndTable();
    }
    ImGui::End();
}

// 2. Renders a detailed window for a single character entity.
void RenderCharacterDetailWindow(
    const flecs::world &ecs,
    const flecs::entity characterEntity,
    const flecs::entity titleEntity,
    const Character& c, const CharacterQueries& queries)
{
    const std::string window_name = c.mName + " - Details";
    bool open = characterEntity.enabled<ShowCharacterDetails>();

    if (ImGui::Begin(window_name.c_str(), &open))
    {
        const auto &character = characterEntity.get<Character>();

        // --- BASIC INFO & DYNASTY ---
        if (ecs.entity<Player>() == characterEntity)
            ImGui::Text("Player Character");

        flecs::entity dynasty_target = characterEntity.target<DynastyMember>();
        auto *d = dynasty_target.try_get<Dynasty>();
        ImGui::Text("Dynasty: %s", d ? d->name.c_str() : "None");

        ImGui::Text("Money: %.2f", character.MoneyFloat());

        ImGui::Text("Age: %zu Y %zu M %zu D", character.mAgeDays / 360, character.mAgeDays / 30 % 12, character.mAgeDays % 30);

        // --- MARRIAGE ---
        flecs::entity spouse_target = characterEntity.target<MarriedTo>();
        if (spouse_target.is_valid())
        {
            auto* spouse = spouse_target.try_get<Character>();
            ImGui::Text("Married To: %s", spouse ? spouse->mName.c_str() : "Single");
        }

        // ImGui::Separator();
        ImGui::Text("Landed Holdings:");

        // --- TITLES & PROVINCES (Hierarchy Traversal) ---
        // Reuse q_ruled_titles to find the direct holdings

        const auto &title = titleEntity.get<Title>();
        if (ImGui::TreeNodeEx(title.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            queries.qProvincesOfTitle
                .set_var("title", titleEntity)
                .each([&](const Province &province, const Title&)
                {
                    ImGui::BulletText("Province: %s (Income: $%.2lf)", province.name.c_str(), province.IncomeFloat());
                });
            ImGui::TreePop();
        }
    }

    if (!open)
    {
        void(characterEntity.disable<ShowCharacterDetails>());
    }

    ImGui::End();
}

flecs::entity BirthChildCharacter(const flecs::world& ecs, const Character& father, const Character& mother, flecs::entity dynasty)
{
    const auto &builder = ecs.get<CharacterBuilder>();
    bool isMale = Random::GetIntRange(0, 1);
    auto name = isMale ? builder.GenMaleName() : builder.GenFemaleName();
    return builder.CreateCharacter(name, dynasty, isMale);
}
