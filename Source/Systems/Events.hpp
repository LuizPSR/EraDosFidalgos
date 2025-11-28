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
