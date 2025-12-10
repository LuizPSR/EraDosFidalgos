#pragma once
#include <cstdint>

struct DiplomacyModule
{
    explicit DiplomacyModule(const flecs::world &ecs);
};

struct RealmRelation
{
    int8_t relations = 0;
};

struct War
{
    uint32_t mStartDate;
};

struct DiploEventChoice
{
    std::string mText;
    int8_t mRelationChange;
};

struct DiploEvent
{
    std::string mTitle;
    std::string mMessage;
    std::vector<DiploEventChoice> mChoices;
};

struct Neighboring {};
