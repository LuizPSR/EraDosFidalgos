#pragma once
#include <flecs.h>

constexpr uint64_t YEAR_DURATION = 360 * 86400;
constexpr uint64_t MONTH_DURATION = 30 * 86400;
constexpr uint64_t WEEK_DURATION = 7 * 86400;
constexpr uint64_t DAY_DURATION = 1 * 86400;

struct GameTime
{
    uint64_t mTimeSecs = 0;
    uint64_t mLastTimeSecs = 0;
    float mSpeed = 0, mSpeedAccel = 0;

    [[nodiscard]] uint64_t CountYearChanges() const
    {
        return mTimeSecs / YEAR_DURATION - mLastTimeSecs / YEAR_DURATION;
    }
    [[nodiscard]] uint64_t CountMonthChanges() const
    {
        return mTimeSecs / MONTH_DURATION - mLastTimeSecs / MONTH_DURATION;
    }
    [[nodiscard]] uint64_t CountWeekChanges() const
    {
        return mTimeSecs / WEEK_DURATION - mLastTimeSecs / WEEK_DURATION;
    }
    [[nodiscard]] uint64_t CountDayChanges() const
    {
        return mTimeSecs / DAY_DURATION - mLastTimeSecs / DAY_DURATION;
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
