#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "Game.h"
#include "Random.h"
#include "Components/Camera.h"
#include "Renderer/Renderer.h"
#include "Systems/ChessBoard.h"

bool Initialize(const flecs::world &ecs)
{
    Random::Init();

    // Init window
    void(ecs.component<Window>()
        .add(flecs::Singleton)
        .emplace<Window>());
    auto &window = ecs.get_mut<Window>();
    if (window.Initialize() == false)
        return false;

    // Init renderer
    void(ecs.component<Renderer>()
        .add(flecs::Singleton)
        .emplace<Renderer>());
    auto &renderer = ecs.get_mut<Renderer>();
    if (renderer.Initialize(window) == false)
        return false;

    void(ecs.entity<InputState>()
        .add(flecs::Singleton)
        .add<InputState>());

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    if (ImGui_ImplSDL3_InitForOpenGL(window.sdlWindow, renderer.mContext) == false)
        return false;
    if (ImGui_ImplOpenGL3_Init("#version 330") == false)
        return false;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Init ECS
    RegisterSystems(ecs);

    return true;
}

void RegisterSystems(const flecs::world &ecs)
{
    ecs.system("ReadInput")
        .kind(flecs::OnLoad)
        .run([](const flecs::iter &it) {
            ProcessInput(it.world());
        });
    ecs.system("StartFrame")
        .kind(flecs::PreUpdate)
        .run([](flecs::iter &)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        });
    ecs.system("UpdateUI")
        .kind(flecs::OnUpdate)
        .run([](flecs::iter &it)
        {
            ImGui::Begin("Menu");
            ImGui::Text("Application Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Separator();
            if (ImGui::Button("Click Me"))
            {
                SDL_Log("I've been clicked!");
            }

            glm::vec2 mPos;
            SDL_GetRelativeMouseState(&mPos.x, &mPos.y);
            ImGui::Text("Mouse Relative: %.1f %.1f", mPos.x, mPos.y);

            static bool chessboardActive = false;
            if (ImGui::Checkbox("Chessboard", &chessboardActive))
            {
                const flecs::world &ecs = it.world();
                if (!chessboardActive)
                {
                    ChessBoardNS::Destroy(ecs);
                } else
                {
                    ChessBoardNS::Initialize(ecs);
                }
            }

            ImGui::End();
        });
    ecs.system<Renderer>("StartRender")
        .kind(flecs::PreStore)
        .each([](Renderer &renderer)
        {
            renderer.Clear();
        });

    ChessBoardNS::RegisterSystems(ecs);

    ecs.system<Renderer>("PresentRender")
        .kind(flecs::OnStore)
        .each([](Renderer &renderer)
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            renderer.Present();
        });
}

void ProcessInput(const flecs::world &ecs)
{
    auto &renderer = ecs.get_mut<Renderer>();
    auto &input = ecs.get_mut<InputState>();

    input.MouseDelta = glm::vec2{0.0f};
    input.MouseScrollAmount = 0.0f;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);

        const auto &io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        {
            if (event.type != SDL_EVENT_QUIT && event.type != SDL_EVENT_WINDOW_RESIZED)
            {
                continue;
            }
        }
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                ecs.quit();
                break;
            case SDL_EVENT_WINDOW_RESIZED:
                {
                    const int width = event.window.data1;
                    const int height = event.window.data2;
                    renderer.UpdateOrthographicMatrix(width, height);
                }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    input.IsRightMouseButtonDown = true;
                    SDL_SetWindowRelativeMouseMode(renderer.mWindow, true);
                } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    input.IsMiddleMouseButtonDown = true;
                    SDL_SetWindowRelativeMouseMode(renderer.mWindow, true);
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP:
            {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    input.IsRightMouseButtonDown = false;
                    if (!input.IsMiddleMouseButtonDown) {
                        SDL_SetWindowRelativeMouseMode(renderer.mWindow, false);
                    }
                } else if (event.button.button == SDL_BUTTON_MIDDLE) {
                    input.IsMiddleMouseButtonDown = false;
                    if (!input.IsRightMouseButtonDown) {
                        SDL_SetWindowRelativeMouseMode(renderer.mWindow, false);
                    }
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION:
            {
                if (input.IsRightMouseButtonDown || input.IsMiddleMouseButtonDown)
                {
                    // Use relative motion for panning
                    input.MouseDelta.x = (float)event.motion.xrel;
                    input.MouseDelta.y = (float)event.motion.yrel;
                }
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL:
            {
                auto scrollAmount = (float)event.wheel.y;
                input.MouseScrollAmount = scrollAmount;
                break;
            }
            default:
                break;
        }
    }

    const bool* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE]) {
        ecs.quit();
    }
}
