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
    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    SDL_Window *window = SDL_CreateWindow("Era dos Fidalgos", SDL_WINDOW_MAXIMIZED, SDL_WINDOW_MAXIMIZED, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    SDL_SetWindowMinimumSize(window, 400, 300);

    ecs.component<Renderer>()
        .add(flecs::Singleton)
        .emplace<Renderer>(window);
    auto &renderer = ecs.get_mut<Renderer>();
    if (renderer.Initialize() == false)
    {
        return false;
    }

    ecs.entity<InputState>()
        .add(flecs::Singleton)
        .add<InputState>();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForOpenGL(window, renderer.mContext);
    ImGui_ImplOpenGL3_Init("#version 330");

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

            const flecs::world &ecs = it.world();
            if (ImGui::Button("Create Scene"))
            {
                ChessBoardNS::Initialize(ecs);
            }
            if (ImGui::Button("Destroy Scene"))
            {
                ChessBoardNS::Destroy(ecs);
            }

            it.world().each([](Camera &camera)
            {
                ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.mPosition.x, camera.mPosition.y, camera.mPosition.z);
                ImGui::Text("Target Position: (%.1f, %.1f, %.1f)", camera.mTarget.x, camera.mTarget.y, camera.mTarget.z);
                ImGui::SliderFloat("Zoom", &camera.mZoomLevel, camera.mMinZoom, camera.mMaxZoom);
            });

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

    input.MouseDeltaX = 0.0f;
    input.MouseDeltaY = 0.0f;
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
                    input.MouseDeltaX = (float)event.motion.xrel;
                    input.MouseDeltaY = (float)event.motion.yrel;
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
