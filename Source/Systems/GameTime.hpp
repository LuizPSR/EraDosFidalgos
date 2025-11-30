#pragma once
#include <flecs.h>

struct GameTime
{
    explicit GameTime(const flecs::world &ecs);

    // Timers
    // Disabled on pause
    flecs::timer mTickTimer;
    // Run every day (MUST get amount of days elapsed from GameTime struct)
    flecs::timer mDayTimer;
    flecs::timer mWeekTimer;
    flecs::timer mMonthTimer;

    uint64_t mTimeSecs = 0;
    uint64_t mLastTimeSecs = 0;
    float mSpeed = 0, mSpeedAccel = 0;

    [[nodiscard]] uint64_t CountMonthChanges() const
    {
        return mTimeSecs / 86400 / 30 - mLastTimeSecs / 86400 / 30;
    }
    [[nodiscard]] uint64_t CountWeekChanges() const
    {
        return mTimeSecs / 86400 / 7 - mLastTimeSecs / 86400 / 7;
    }
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

void DoGameTimeSystems(const flecs::world& ecs, flecs::timer tickTimer);
