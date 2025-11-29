#pragma once
#include <string>
#include <flecs.h>

struct Title
{
    std::string name;
};

struct Character
{
    std::string name;
    uint64_t money;
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

struct CharactersModule
{
    explicit CharactersModule(const flecs::world &ecs);
};

void RenderCharacterOverviewWindow(
    const flecs::world& ecs,
    const flecs::query<const Character> &qRulers,
    const flecs::query<> &qInRealm);

void RenderCharacterDetailWindow(
    const flecs::world &ecs,
    flecs::entity character,
    const flecs::query<>& qInRealm,
    const Character& c);