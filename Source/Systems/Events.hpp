#pragma once
#include <flecs.h>
#include <string>

struct EventsSampleScene
{
    explicit EventsSampleScene(const flecs::world &ecs);
};

struct GameTime
{
    uint64_t mTimeSecs = 0;
    uint64_t mLastTimeSecs = 0;
    float mSpeed = 0, mSpeedAccel = 0;

    [[nodiscard]] uint64_t CountDayChanges() const
    {
        return mTimeSecs / 86400 - mLastTimeSecs / 86400;
    }
    [[nodiscard]] uint64_t TimeDays() const
    {
        return mTimeSecs / 86400;
    }
    [[nodiscard]] uint64_t TimeHours() const
    {
        return (mTimeSecs / 3600) % 24;
    }
    [[nodiscard]] uint64_t TimeMinutes() const
    {
        return (mTimeSecs / 60) % 60;
    }
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

struct PlanMarriage
{
    flecs::entity character;
};

struct PregnancySaga
{
    enum class Stage
    {
        Attempt,
        Announce,
        Birth,
    } stage;
    flecs::entity father, mother;
};
