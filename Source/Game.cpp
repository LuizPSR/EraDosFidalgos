#include <algorithm>
#include <fstream>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <string>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

#include "Game.hpp"
#include "Random.hpp"
#include "ImGUIConfig.hpp"
#include "Renderer/Renderer.hpp"

// UI Screens
#include "UI/UIScreens/MainMenu.hpp"
#include "UI/UIScreens/PauseMenu.hpp"
#include "UI/UIScreens/GameUIModule.hpp"
#include "UI/UIScreens/UICommon.hpp" // Adicionar este include

// Systems
#include "Systems/GameBoard.hpp"
#include "Systems/Events.hpp"
#include "Systems/Characters.hpp"
#include "Systems/Diplomacy.hpp"
#include "Systems/Sound.hpp"
#include "Systems/MapGenerator.hpp"
#include "Systems/ProvinceUpdate.hpp"
#include "UI/GameOver.hpp"

bool Initialize(flecs::world &ecs) {
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

    void(ecs.component<Camera>()
        .add(flecs::Singleton));

    void(ecs.component<GameTickSources>()
        .add(flecs::Singleton)
        .emplace<GameTickSources>(ecs));

    // Creates ImGUI ini file path
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

    // Init ECS
    RegisterSystems(ecs);
    ImportModules(ecs);

    return true;
}

void RegisterSystems(flecs::world &ecs) {
    void(ecs.component<GameStarted>().add(flecs::Singleton));
    void(ecs.entity<GameEnded>().add(flecs::Singleton));

    const auto playerCapital = ecs.query_builder<const Province>("PlayerCapital")
        .with<CapitalOf>("$title")
        .with<RulerOf>("$title").src<Player>()
        .build();

    ecs.system<GameTickSources>("SetupGame")
        .with<GameStarted>()
        .immediate()
        .each([=](flecs::iter, size_t, GameTickSources &tickSources)
        {
            ecs.defer_suspend();

            const auto oldScope = ecs.set_scope(ecs.entity("Kingdoms"));
            void(ecs.entity<Player>().add<Player>());
            void(ecs.add<GameTime>());
            void(ecs.add<EstatePowers>());
            void(ecs.add<Camera>());
            GenerateMap(ecs, 36533);
            CreateKingdoms(ecs);
            void(ecs.set_scope(oldScope));

            auto &camera = ecs.get_mut<Camera>();

            tickSources.mTickTimer.start();

            playerCapital.each([&](const Province &capital)
            {
                camera.mTarget = glm::vec2(capital.mPosX, capital.mPosY) * 32.0f;
            });
            void(ecs.remove<GameStarted>());

            ecs.defer_resume();
        });

    ecs.system<>("DestroyGame")
        .with<GameEnded>()
        .each([=](flecs::iter, size_t)
        {
            ecs.entity<Player>().clear();
            ecs.remove<GameTime>();
            ecs.remove<EstatePowers>();
            ecs.remove<Camera>();
            ecs.entity("Kingdoms").destruct();
            ecs.entity("Events").destruct();
            ecs.remove<GameEnded>();
        });

    ecs.system("ReadInput")
        .kind(flecs::OnLoad)
        .run([](const flecs::iter &it) {
            ProcessInput(it.world());
        });
    ecs.system("CleanupOnStart")
        .kind(flecs::OnLoad)
        .run([](const flecs::iter &it) {
            // Remover entidade GameOver se existir
            auto gameOverEntity = it.world().lookup("GameOver");
            if (gameOverEntity.is_valid()) {
                gameOverEntity.destruct();
            }
        });
    ecs.system("StartFrame")
        .kind(flecs::PreUpdate)
        .run([](flecs::iter &) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();

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
        .each([](Renderer &renderer) {
            renderer.Clear();
        });

    ecs.system<Renderer>("PresentRender")
        .kind(flecs::OnStore)
        .each([](Renderer &renderer) {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            renderer.Present();
        });
}

void ImportModules(flecs::world& ecs) {
    // UI Modules
    void(ecs.import<MainMenuModule>());
    void(ecs.import<PauseMenuModule>().disable());
    void(ecs.import<GameUIModule>().disable());
    void(ecs.import<GameOverModule>().disable());  // Adicionar esta linha

    flecs::entity gameUI = ecs.entity<GameUIModule>();

    // Game Systems
    void(ecs.import<SoundModule>().disable());
    void(ecs.import<CharactersModule>().child_of(gameUI));
    void(ecs.import<GameBoardScene>().child_of(gameUI));
    void(ecs.import<EventsModule>().child_of(gameUI));
    void(ecs.import<DiplomacyModule>().child_of(gameUI));
    void(ecs.import<ProvinceUpdates>().child_of(gameUI));
}

GameTickSources::GameTickSources(const flecs::world& ecs) {
    mTickTimer = ecs.timer("TickTimer");
    mDayTimer = ecs.timer("DayTimer");
    mWeekTimer = ecs.timer("WeekTimer");
    mMonthTimer = ecs.timer("MonthTimer");
    mYearTimer = ecs.timer("YearTimer");

    // Iniciar apenas o tick timer, os outros podem ficar parados inicialmente
    mDayTimer.stop();
    mWeekTimer.stop();
    mMonthTimer.stop();
    mYearTimer.stop();
}

void ProcessInput(const flecs::world &ecs)
{
    auto &renderer = ecs.get_mut<Renderer>();
    auto &input = ecs.get_mut<InputState>();

    input.MouseDelta = glm::vec2{0.0f};
    input.MouseScrollAmount = 0.0f;
    input.WasEscapePressed = false;
    input.Clicked = false;

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
                if (event.button.button == SDL_BUTTON_LEFT) {
                    input.Clicked = true;
                }
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
