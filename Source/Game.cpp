#include <algorithm>
#include <fstream>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

#include "Game.hpp"
#include "Random.hpp"
#include "ImGUIConfig.hpp"
#include "Renderer/Renderer.hpp"
#include "Systems/ChessBoard.hpp"
#include "Systems/Events.hpp"
#include "Systems/Characters.hpp"
#include "Systems/Sound.hpp"

#include "Systems/MapGenerator.hpp"

struct MainMenuModule
{
    explicit MainMenuModule(flecs::world &ecs);
};

struct PauseMenuModule
{
    explicit PauseMenuModule(flecs::world &ecs);
};

void UnPause(const flecs::world &ecs, GameTickSources &timers)
{
    void(timers.mTickTimer.start());
}

void Pause(const flecs::world &ecs, GameTickSources &timers)
{
    void(timers.mTickTimer.stop());
}

bool HorizontalButton(const char* label, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0))
{
    return ImGui::Button(label, size_arg);
}

struct TestUIModule
{
    explicit TestUIModule(flecs::world &ecs);
};

MainMenuModule::MainMenuModule(flecs::world& ecs)
{
    ecs.system<GameTickSources, const InputState>("MainMenu")
        .each([](const flecs::iter &it, size_t, GameTickSources &timers, const InputState &input)
        {
            const auto &ecs = it.world();

            const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            const ImVec2 size = ImGui::GetMainViewport()->Size;
            ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
            ImGui::SetNextWindowSizeConstraints(ImVec2{size.x * 0.6f, size.y * 0.2f}, ImVec2{FLT_MAX, FLT_MAX});
            if (ImGui::Begin("Era dos Fidalgos", nullptr,
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground))
            {
                if (HorizontalButton("Start"))
                {
                    void(ecs.entity<MainMenuModule>().disable());
                    void(ecs.entity<TestUIModule>().enable());
                    UnPause(ecs, timers);
                }
                if (HorizontalButton("Quit") || input.WasEscapePressed)
                    ecs.quit();
            }
            ImGui::End();
        });
}

PauseMenuModule::PauseMenuModule(flecs::world& ecs)
{
    ecs.system<GameTickSources, const InputState>("PauseMenu")
       .each([](const flecs::iter &it, size_t, GameTickSources &timers, const InputState &input)
       {
           const auto &ecs = it.world();

           const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
           const ImVec2 size = ImGui::GetMainViewport()->Size;
           ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
           ImGui::SetNextWindowSizeConstraints(ImVec2{size.x * 0.6f, size.y * 0.2f}, ImVec2{FLT_MAX, FLT_MAX});
           if (ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize))
           {
               if (HorizontalButton("Resume") || input.WasEscapePressed)
               {
                   void(ecs.entity<PauseMenuModule>().disable());
                   void(ecs.entity<TestUIModule>().enable());
                   UnPause(ecs, timers);
               }
               if (HorizontalButton("Quit"))
               {
                   void(ecs.entity<TestUIModule>().disable());
                   void(ecs.entity<PauseMenuModule>().disable());
                   void(ecs.entity<MainMenuModule>().enable());
               }
           }
           ImGui::End();
       });
}

TestUIModule::TestUIModule(flecs::world& ecs)
{
    const flecs::entity tickTimer = ecs.get<GameTickSources>().mTickTimer;

    ecs.system<GameTickSources>("UpdateUI")
        .tick_source(tickTimer)
        .kind(flecs::OnUpdate)
        .each([](const flecs::iter &it, size_t, GameTickSources &timers)
        {
            const auto &ecs = it.world();
            if (auto input = ecs.try_get<InputState>(); input->WasEscapePressed)
            {
                Pause(ecs, timers);
                void(ecs.entity<PauseMenuModule>().enable());
            }
            if (ImGui::Begin("Menu"))
            {
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
                    if (!chessboardActive)
                    {
                        ChessBoardScene::Stop(ecs.entity<ChessBoardScene>().disable());
                    } else
                    {
                        ChessBoardScene::Start(ecs.entity<ChessBoardScene>().enable());
                    }
                }

                static bool showDemo = false;
                ImGui::Checkbox("Show ImGUI Demo", &showDemo);
                if (showDemo) ImGui::ShowDemoWindow();

                if (ImGui::Button("Create Kingdoms"))
                {
                    CreateKingdoms(ecs, 3);
                }
            }
            ImGui::End();
        });
}

bool Initialize(flecs::world &ecs)
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

    // Creates ImGUI ini file path
    // Has to have a static lifetime
    static char iniPathBuf[256] = {};
    const auto iniPath = std::filesystem::path(SDL_GetPrefPath("dcc_jogos", "EraDosFidalgos")) / "imgui.ini";
    strncpy(iniPathBuf, iniPath.string().data(), 255);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SetupImGuiStyle();
    SetupImguiFlags(iniPathBuf);

    // Setup Platform/Renderer backends
    if (ImGui_ImplSDL3_InitForOpenGL(window.sdlWindow, renderer.mContext) == false)
        return false;
    if (ImGui_ImplOpenGL3_Init("#version 330") == false)
        return false;

    // Game Tick Sources
    void(ecs.component<GameTickSources>()
        .add(flecs::Singleton)
        .emplace<GameTickSources>(GameTickSources{ecs}));

    // Init ECS
    RegisterSystems(ecs);
    ImportModules(ecs);

    return true;
}

void RegisterSystems(flecs::world &ecs)
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

            // Limit UI screen space
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            float borderThickness = 16.0f;
            viewport->WorkPos.x += borderThickness;
            viewport->WorkPos.y += borderThickness;
            viewport->WorkSize.x -= (2 * borderThickness);
            viewport->WorkSize.y -= (2 * borderThickness);

            ImGui::DockSpaceOverViewport(0, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
        });
    ecs.system<Renderer>("StartRender")
        .kind(flecs::PreStore)
        .each([](Renderer &renderer)
        {
            renderer.Clear();
        });
    ecs.system<Renderer>("PresentRender")
        .kind(flecs::OnStore)
        .each([](Renderer &renderer)
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            renderer.Present();
        });

    // Register the map generation system
    RegisterMapGenerationSystem(ecs);

    // Trigger map generation by creating an entity with GenerateMap component
    ecs.entity()
        .set<GenerateMap>({42});  // seed = 42
}

void ImportModules(flecs::world& ecs)
{
    void(ecs.import<MainMenuModule>());

    void(ecs.import<PauseMenuModule>().disable());

    void(ecs.import<TestUIModule>().disable());

    void(ecs.import<SoundModule>().disable());

    void(ecs.import<CharactersModule>()
        .child_of<TestUIModule>());

    void(ecs.import<ChessBoardScene>()
        .child_of<TestUIModule>()
        .disable());

    void(ecs.import<EventsSampleScene>()
        .child_of<TestUIModule>());
}

GameTickSources::GameTickSources(const flecs::world& ecs)
{
    mTickTimer = ecs.timer("TickTimer");
    mDayTimer = ecs.timer("DayTimer");
    mDayTimer.stop();
    mWeekTimer = ecs.timer("WeekTimer");
    mWeekTimer.stop();
    mMonthTimer = ecs.timer("MonthTimer");
    mMonthTimer.stop();
    mYearTimer = ecs.timer("YearTimer");
    mYearTimer.stop();
}

void ProcessInput(const flecs::world &ecs)
{
    auto &renderer = ecs.get_mut<Renderer>();
    auto &input = ecs.get_mut<InputState>();

    input.MouseDelta = glm::vec2{0.0f};
    input.MouseScrollAmount = 0.0f;
    input.WasEscapePressed = false;

    const auto &io = ImGui::GetIO();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);

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
                if (io.WantCaptureMouse) break;
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
                if (io.WantCaptureMouse) break;
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
                if (io.WantCaptureMouse) break;
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
                if (io.WantCaptureMouse) break;
                auto scrollAmount = (float)event.wheel.y;
                input.MouseScrollAmount = scrollAmount;
                break;
            }
            case SDL_EVENT_KEY_UP:
            {
                if (event.key.key == SDLK_F11 && !io.WantCaptureKeyboard)
                {
                    auto *window = ecs.get<Window>().sdlWindow;
                    bool isFullScreen = SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN;
                    SDL_SetWindowFullscreen(window, !isFullScreen);
                    SDL_SetWindowMouseGrab(window, isFullScreen);
                }
                if (event.key.key == SDLK_ESCAPE)
                {
                    input.WasEscapePressed = true;
                }
            }
            default:
                break;
        }
    }
}
