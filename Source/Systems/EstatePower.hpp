#pragma once
#include "Events.hpp"
#include "Game.hpp"

struct EstatePowers
{
    int8_t mCommonersPower = 0;
    int8_t mNobilityPower = 0;
    int8_t mClergyPower = 0;
};

enum class SocialEstate
{
    Commoners,
    Nobility,
    Clergy,
};

struct EstateEventChoice
{
    std::string mText;
    std::vector<std::pair<SocialEstate, int>> mPowerChanges;
    uint64_t mCost;

    [[nodiscard]] float FloatCost() const
    {
        return float(mCost) * 0.01f;
    }
};

struct EstatePowerEvent
{
    std::string mMessage;
    std::vector<EstateEventChoice> mChoices;
};

void DoEstatePowerSystems(const flecs::world& ecs, const GameTime& timers);
