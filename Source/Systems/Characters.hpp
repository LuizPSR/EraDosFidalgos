#pragma once
#include <string>
#include <flecs.h>
#include <glm/glm.hpp>

#include "Components/Province.hpp"
#include "Components/Culture.hpp"

struct Title
{
    std::string name;
    glm::vec3 color;
};

struct Character
{
    std::string mName;
    uint64_t mMoney = 0;
    uint64_t mAgeDays = 0;

    // Transforms income from fixed point to floating point (USE FOR DISPLAY ONLY)
    [[nodiscard]] double MoneyFloat() const
    {
        return static_cast<double>(mMoney) * 0.01;
    }
};

// -- RELATIONS --

struct CharacterCulture {CultureType culture;};

struct RulerOf {};

struct Courtier {};

struct MarriedTo {};

struct DynastyMember {};

struct DynastyHead {};

// -- TAG --
// Whether to show character details
struct ShowCharacterDetails {};

struct Deceased {};

enum class Gender
{
    Male, Female
};

enum class AgeClass
{
    Child, Adult
};

struct Player {};

// -- Module --

struct CharactersModule
{
    explicit CharactersModule(const flecs::world &ecs);
};

struct CharacterQueries
{
    // Finds all characters with a title
    flecs::query<const Character, const Title> qAllRulers;
    // Finds all provinces under a title
    flecs::query<const Province, const Title> qProvincesOfTitle;
    // Find all characters belonging to a dynasty
    flecs::query<const Character> qMembersOfDynasty;
};

void CreateKingdoms(const flecs::world &ecs);

void RenderCharacterOverviewWindow(
    const flecs::world& ecs, const CharacterQueries& queries);

void RenderRulerRow(
    const flecs::world &ecs,
    const CharacterQueries &queries,
    const Character &character, flecs::entity ruler,
    const Title& title, flecs::entity titleEntity);

void RenderDynastyMemberRow(
    const flecs::world& ecs,
    const CharacterQueries& queries,
    const Character& character, flecs::entity ruler,
    flecs::entity playerDynasty);

void RenderCharacterDetailWindow(
    const flecs::world &ecs,
    flecs::entity characterEntity,
    flecs::entity titleEntity, const Character& c, const CharacterQueries& queries);

flecs::entity BirthChildCharacter(const flecs::world& ecs, const Character& father, const Character& mother,
                                  flecs::entity dynasty);