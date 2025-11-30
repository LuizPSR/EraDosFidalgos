#pragma once
#include <cstdint>
#include <string>

struct Dynasty
{
    std::string name;
};

struct DynastyRelations
{
    // 0 to 255
    uint8_t relation;
};
