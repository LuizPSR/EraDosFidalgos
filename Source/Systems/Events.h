#pragma once
#include <flecs.h>
#include <string>

struct EventsSampleScene
{
    explicit EventsSampleScene(const flecs::world &ecs);
};

struct GameTime
{
    float mTime = 0, mSpeed = 0, mSpeedAccel = 0;
};

struct EventSchedule
{
    float mTime;
    bool operator<(const EventSchedule &o) const
    {
        return mTime < o.mTime;
    }
};

struct FiredEvent {};

struct SamplePopup
{
    std::string mMessage;
};

// TODO: Organize the below structs

struct Title
{
    std::string name;
};

struct Province
{
    std::string name;
    float income;
};

struct Character
{
    std::string name;
};

struct Dynasty
{
    std::string name;
};

// Character to Title
struct RulerOf {};

// Character to Title
struct CourtierOf {};

// Character to Character
struct MarriedTo {};

// Character to Dynasty
struct DynastyMember {};

// Province to Title or Title to Title
struct InRealm {};

// Whether to show character details
struct ShowDetails {};

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