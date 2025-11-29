#pragma once
#include <string>
#include <cstdint>

struct InRealm {};

struct Province
{
    std::string name;
    uint64_t income;

    // Transforms income from fixed point to floating point (USE FOR DISPLAY ONLY)
    [[nodiscard]] double IncomeFloat() const
    {
        return static_cast<double>(income) * 0.01;
    }
};