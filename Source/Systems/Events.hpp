#pragma once
#include <flecs.h>
#include <string>

#include "GameTime.hpp"
#include "Random.hpp"

// TODO: initialize this entity in game
struct EventsRng: Random {};

struct EventsModule
{
    explicit EventsModule(const flecs::world &ecs);
};

struct EventSchedule
{
    uint64_t mTimeSecs;
    bool operator<(const EventSchedule &o) const
    {
        return mTimeSecs < o.mTimeSecs;
    }

    static EventSchedule AtDay(const uint64_t day)
    {
        return { day * 86400 };
    }
    static EventSchedule InXDays(const GameTime &time, uint64_t days)
    {
        return { time.mTimeSecs + days * 86400 };
    }
};

struct FiredEvent {};

struct PausesGame {};

struct SamplePopup
{
    std::string mMessage;
};

struct PlanMarriageSaga
{
    flecs::entity character;
};

struct PregnancySaga
{
    enum Stage
    {
        Attempt,
        Announce,
        Birth,
        BirthAnnounce,
    } stage = Attempt;
    flecs::entity father, mother, dynasty, child;
};
