#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>

struct Window
{
    ~Window();

    SDL_Window *sdlWindow = nullptr;

    bool Initialize();

    glm::vec2 GetSize() const;
    glm::vec2 GetMousePosNDC() const;
};
