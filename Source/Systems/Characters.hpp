#pragma once
#include <string>
#include <flecs.h>

struct Title
{
    std::string name;
};

struct Character
{
    std::string mName;
    uint64_t mMoney = 0.0f;
    uint64_t mAgeDays = 0;

    // Transforms income from fixed point to floating point (USE FOR DISPLAY ONLY)
    [[nodiscard]] double MoneyFloat() const
    {
        return static_cast<double>(mMoney) * 0.01;
    }
};

struct Dynasty
{
    std::string name;
};

// -- RELATIONS --
// Character to Title / Symmetric
struct Ruler {};

// Character to Title / Symmetric
struct Courtier {};

// Character to Character / Symmetric
struct MarriedTo {};

// Character to Dynasty / Symmetric
struct DynastyMember {};

// Dynasty to Character / Symmetric
struct DynastyHead {};


// -- TAG --
// Whether to show character details
struct ShowCharacterDetails {};

struct Male {};

struct Female {};

struct Adult {};

struct Player {};

// -- Module --

struct CharactersModule
{
    explicit CharactersModule(const flecs::world &ecs);
};

void CreateKingdoms(const flecs::world &ecs, size_t count);

void RenderCharacterOverviewWindow(
    const flecs::world& ecs,
    const flecs::query<const Character> &qRulers,
    const flecs::query<> &qInRealm);

void RenderCharacterDetailWindow(
    const flecs::world &ecs,
    flecs::entity characterEntity,
    const flecs::query<>& qInRealm,
    const Character& c);