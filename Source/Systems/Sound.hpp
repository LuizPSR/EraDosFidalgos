#pragma once

#include <flecs.h>

struct SoundMixer
{
};

struct SoundModule
{
    explicit SoundModule(const flecs::world &ecs);
};