#include "Window.h"

bool Window::Initialize()
{
    SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "1");

    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    sdlWindow = SDL_CreateWindow("Era dos Fidalgos", SDL_WINDOW_MAXIMIZED, SDL_WINDOW_MAXIMIZED, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!sdlWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    SDL_SetWindowMouseGrab(sdlWindow, true);
    SDL_SetWindowMinimumSize(sdlWindow, 400, 300);

    return true;
}

glm::vec2 Window::GetSize() const
{
    int width, height;
    SDL_GetWindowSize(sdlWindow, &width, &height);
    return glm::vec2{width, height};
}

glm::vec2 Window::GetMousePosNDC() const
{
    const glm::vec2 sz = GetSize();
    glm::vec2 mousePos;
    SDL_GetMouseState(&mousePos.x, &mousePos.y);
    mousePos.x = 2.0f * mousePos.x / sz.x - 1.0f;
    mousePos.y = 1.0f - 2.0f * mousePos.y / sz.y;
    return mousePos;
}
