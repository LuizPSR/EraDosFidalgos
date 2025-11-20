//
//  Main.cpp
//  Game-mac
//
//  Created by Sanjay Madhav on 5/31/17.
//  Copyright © 2017 Sanjay Madhav. All rights reserved.
//

#include <filesystem>
#include <SDL3/SDL.h>
#include <thread>

#include "Game.h"

int main(int argc, char* argv[])
{
    std::filesystem::path exePath = std::filesystem::canonical(argv[0]).parent_path();
    std::filesystem::current_path(exePath);

    flecs::world ecs(argc, argv);
    if (Initialize(ecs) == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to initialize game");
        return 1;
    }
    const auto numThreads = std::thread::hardware_concurrency();
    return ecs.app()
        .enable_rest()
        .enable_stats()
        .threads(static_cast<int>(numThreads))
        .run();
}
