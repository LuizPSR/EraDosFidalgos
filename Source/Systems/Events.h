#pragma once
#include <flecs.h>
#include <string>

struct EventsSampleScene
{
    explicit EventsSampleScene(const flecs::world &ecs);
};

struct GameTime
{
    size_t mDay;
};

struct EventSchedule
{
    size_t mDay;
    bool operator<(const EventSchedule &o) const
    {
        return mDay < o.mDay;
    }
};

struct FiredEvent {};

struct SamplePopup
{
    std::string mMessage;
};
