#include <imgui.h>
#include <flecs.h>
#include <toml++/toml.hpp>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL.h>
#include <filesystem>

#include "Characters.hpp"

#include "Game.hpp"
#include "Components/Province.hpp"
#include "Components/Culture.hpp"
#include "MapGenerator.hpp"

#include "Components/Dynasty.hpp"
// TODO: make random deterministic
#include "GameTime.hpp"
#include "Random.hpp"
#include "UI/UIScreens/GameUI.hpp"

struct EntityHandler {
    flecs::entity ruler;
    flecs::entity dynasty;
    flecs::entity kingdom;
    CultureType culture;
};

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
    CultureType GenCulture() const;

    EntityHandler CreateDynastyWithKingdomAndFamily(bool isPlayer = false) const;
    flecs::entity CreateCharacter(
        const std::string& char_name,
        const flecs::entity& dynasty_entity,
        bool isMale,
        flecs::entity baseEntity,
        CultureType culture
        ) const;
    flecs::entity CreateCharacter(
        const std::string& char_name,
        const flecs::entity& dynasty_entity,
        bool isMale,
        CultureType culture
        ) const
    {
        return CreateCharacter(char_name, dynasty_entity, isMale, ecs.entity(), culture);
    }
};

flecs::entity CharacterBuilder::CreateCharacter(
    const std::string& char_name,
    const flecs::entity& dynasty_entity,
    bool isMale,
    flecs::entity baseEntity,
    CultureType culture
    ) const
{
    flecs::entity character = baseEntity
        .set<Character>({
            .mName = char_name,
            .mMoney = 4971,
            .mAgeDays = 20 * 360,
        });

    if (dynasty_entity.is_valid())
        void(character.add<DynastyMember>(dynasty_entity));

    void(character.set<CharacterCulture>({culture}));
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

CultureType CharacterBuilder::GenCulture() const
{
    switch (Random::GetIntRange(1, 4)) {
    case 0:
        return SteppeNomads;
    case 1:
        return HillDwellers;
    case 2:
        return ForestFolk;
    default:
        return FarmLanders;
    }
}

EntityHandler CharacterBuilder::CreateDynastyWithKingdomAndFamily(bool isPlayer) const
{
    auto dName = GenDynastyName();

    auto dynasty = ecs.entity().set<Dynasty>({ dName });

    auto kingdom = ecs.entity().set<Title>({ dName + " Kingdom" });
    auto culture = GenCulture();

    void(dynasty.set<CharacterCulture>(CharacterCulture { culture }));


    auto rulerName = GenMaleName();
    auto ruler = CreateCharacter(rulerName, dynasty, true, isPlayer ? ecs.entity<Player>() : ecs.entity(), culture)
    .add<DynastyHead>(dynasty).add<CharacterCulture>(culture);
    void(kingdom.add<RuledBy>(ruler));

    void(ruler.add<RulerOf>(kingdom));

    auto spouseName = GenFemaleName();
    auto spouse = CreateCharacter(spouseName, dynasty, false, culture)
        .add<MarriedTo>(ruler)
        .add<DynastyMember>(dynasty)
        .add<CharacterCulture>(GenCulture());
    void(ruler.add<MarriedTo>(spouse));

    return EntityHandler {
        ruler,
        dynasty,
        kingdom,
        culture
    };
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
    const flecs::entity tickTimer = ecs.get<GameTickSources>().mTickTimer;

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

void CreateKingdoms(const flecs::world &ecs)
{
    // Local structure for the priority queue in the Dijkstra-like algorithm
    struct ProvinceDistance
    {
        flecs::entity province_entity;
        float distance; // Accumulated distance from the seed

        // Min-heap ordering (std::priority_queue is max-heap by default, so use operator>)
        bool operator>(const ProvinceDistance& other) const
        {
            return distance > other.distance;
        }
    };

    struct EntityHash {
        size_t operator()(const flecs::entity& e) const {
            // Use the underlying ID of the entity for hashing.
            return std::hash<uint64_t>{}(e.id());
        }
    };

    // Map to hold all unruled, non-sea provinces still available for a new kingdom seed
    std::unordered_map<flecs::entity, bool, EntityHash> available_provinces;
    std::unordered_map<flecs::entity, float, EntityHash> current_expansion_distance;

    // --- 1. Identify all initial unruled, non-sea provinces ---

    flecs::entity tilemap_entity = ecs.lookup("TileMap");
    if (!tilemap_entity.is_valid()) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "CreateKingdoms: TileMap entity not found. Cannot generate kingdoms.");
        return;
    }
    const TileMap* tilemap = &tilemap_entity.get<TileMap>();
    if (!tilemap) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "CreateKingdoms: TileMap component not found. Cannot generate kingdoms.");
        return;
    }

    // Query all province entities to build the initial available pool
    ecs.each<const Province>([&](const flecs::entity p_entity, const Province& p) {
        // Check 1: Is it a non-sea tile?
        if (p.terrain != TerrainType::Sea) {
            // Check 2: Is it unruled? (Check for the target of the RuledBy relationship)
            if (!p_entity.target<RuledBy>().is_valid()) {
                available_provinces[p_entity] = true;
            }
        }
    });

    if (available_provinces.empty()) {
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "No available non-sea provinces to form kingdoms.");
        return;
    }

    const auto& builder = ecs.get<CharacterBuilder>();
    int kingdom_index = 0;

    // --- 2. Kingdom Generation Loop ---
    while (!available_provinces.empty())
    {
        // A. Create New Kingdom (Dynasty/Title)
        auto handler = builder.CreateDynastyWithKingdomAndFamily(kingdom_index++ == 0);

        auto kingdom_dynasty = handler.dynasty;
        auto kingdom_title = handler.kingdom;
        auto kingdom_ruler = handler.ruler;
        auto culture = handler.culture;


        // B. Select Random Seed Province (Initial Capital)
        std::vector<flecs::entity> current_unruled_provinces;
        for (const auto&[fst, snd] : available_provinces) {
            if (snd) { // Only pick truly available provinces
                current_unruled_provinces.push_back(fst);
            }
        }

        if (current_unruled_provinces.empty()) {
            break; // No more provinces available
        }

        flecs::entity seed_province_entity = current_unruled_provinces[Random::GetIntRange(0, current_unruled_provinces.size() - 1)];

        // Temporary maps and queue for the current kingdom's expansion
        std::vector<flecs::entity> newly_claimed_provinces;
        std::priority_queue<ProvinceDistance, std::vector<ProvinceDistance>, std::greater<ProvinceDistance>> pq;

        // 1. Initialize Seed
        pq.push({seed_province_entity, 0.0f});
        current_expansion_distance[seed_province_entity] = 0.0f;

        // Claim seed province
        available_provinces[seed_province_entity] = false; // Mark as no longer available in the pool
        newly_claimed_provinces.push_back(seed_province_entity);

        // 2. Expansion Loop (Dijkstra-like)
        while (!pq.empty())
        {
            ProvinceDistance current = pq.top();
            pq.pop();

            flecs::entity current_entity = current.province_entity;
            float current_dist = current.distance;

            // Get current tile data
            const TileData* current_tile_data = &current_entity.get<TileData>();
            const Province* current_province = &current_entity.get<Province>();

            if (!current_tile_data || !current_province) continue;

            // Cost to move *from* the current tile to a neighbor
            float travel_cost = current_province->movement_cost;

            // Iterate through 4 cardinal directions
            const int dx[] = {0, 0, 1, -1};
            const int dy[] = {1, -1, 0, 0};

            for (int i = 0; i < 4; ++i)
            {
                int nx = current_tile_data->x + dx[i];
                int ny = current_tile_data->y + dy[i];

                // Check bounds
                if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) {
                    continue;
                }

                flecs::entity neighbor_entity = tilemap->tiles[nx][ny];
                if (!neighbor_entity.is_valid()) continue;

                const Province* neighbor_province = &neighbor_entity.get<Province>();
                if (!neighbor_province) continue;

                // Ignore sea tiles
                if (neighbor_province->terrain == TerrainType::Sea) {
                    continue;
                }

                // Calculate new distance using the current tile's movement cost
                float new_dist = current_dist + travel_cost;

                // Stop condition: distance > 100
                if (new_dist > 100.0f) {
                    continue;
                }

                // Check if the tile is unruled (available)
                bool is_available = available_provinces.count(neighbor_entity) && available_provinces.at(neighbor_entity);
                // Check if the tile is already claimed by this kingdom (i.e., in the newly_claimed list)
                bool is_claimed_by_self = std::find(newly_claimed_provinces.begin(), newly_claimed_provinces.end(), neighbor_entity) != newly_claimed_provinces.end();

                // Get the old distance (or infinity if not yet reached)
                float old_dist = current_expansion_distance.count(neighbor_entity)
                               ? current_expansion_distance.at(neighbor_entity)
                               : std::numeric_limits<float>::infinity();

                if (is_available || is_claimed_by_self)
                {
                    if (new_dist < old_dist)
                    {
                        // Found a shorter path
                        current_expansion_distance[neighbor_entity] = new_dist;

                        // Claim it if it was available (only claims unruled tiles)
                        if (is_available) {
                            available_provinces[neighbor_entity] = false; // Mark as no longer available
                            newly_claimed_provinces.push_back(neighbor_entity);
                            neighbor_entity.add<RuledBy>(kingdom_title); // Assign ownership
                        }
                        // Push to queue for further expansion
                        pq.push({neighbor_entity, new_dist});
                    }
                }
            }
        }

        // D. Select Capital (Most Central Province: minimum distance from the initial seed)
        flecs::entity capital_province_entity = flecs::entity::null();
        float min_dist = std::numeric_limits<float>::max();

        for (flecs::entity p_entity : newly_claimed_provinces)
        {
            if (current_expansion_distance.count(p_entity))
            {
                float dist = current_expansion_distance.at(p_entity);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    capital_province_entity = p_entity;
                }
            }
        }

        // E. Set Capital Relation and Update Province Distances
        if (capital_province_entity.is_valid())
        {
            // Set the capital relationship: Province has (CapitalOf, KingdomTitleEntity)
            capital_province_entity.add<CapitalOf>(kingdom_title);


            // Update distance_to_capital for all provinces in the realm
            for (flecs::entity p_entity : newly_claimed_provinces)
            {
                if (Province* p = p_entity.try_get_mut<Province>())
                {
                    // Store the shortest distance from the most central seed as distance_to_capital
                    p->distance_to_capital = current_expansion_distance.count(p_entity)
                                            ? current_expansion_distance.at(p_entity)
                                            : std::numeric_limits<float>::infinity();

                    auto traits = GetCulturalTraits(p->culture);

                    p->name = builder.GenProvinceName();

                    p->popular_opinion = culture == p->culture ? 10 : -10;
                    p->control = 80 +
                        p->popular_opinion -
                        p->distance_to_capital * 0.2f +
                        10 * GetCulturalTraits(culture).extra_control;
                    p->development = Random::GetIntRange(3, 15) + 5 * traits.extra_development;

                    p->income = 5 * (100 + p->development) * p->control * 0.01f;

                    void(p_entity.add<InRealm>(kingdom_title));
                }
            }
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Kingdom %d formed with %zu provinces. Capital set.", kingdom_index, newly_claimed_provinces.size());
        } else {
             SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Kingdom %d generated no provinces. This should not happen if a seed was available.", kingdom_index);
        }
    }
}

// 1. Renders the main window listing all rulers.
void RenderCharacterOverviewWindow(const flecs::world& ecs, const CharacterQueries& queries)
{
    if (ImGui::Begin("Resumo de Personagens"))
    {
        ImGui::BeginTable("RulerListCols", 3);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("Nome do Rei");
        ImGui::TableNextColumn(); ImGui::Text("Título Primário");
        ImGui::TableNextColumn(); ImGui::Text("Ação");

        // Iterate through all Rulers
        queries.qAllRulers
            .each([&](flecs::iter it, size_t i, const Character &c, const Title &title)
            {
                flecs::entity ruler = it.entity(i), titleEntity = it.get_var("title");
                if (ruler == it.world().entity<Player>()) return;

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
            ImGui::Text("Personagem do Jogador");

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
    auto child = builder.CreateCharacter(name, dynasty, isMale,dynasty.get<CharacterCulture>().culture);

    return child;
}
