#include <algorithm>
#include <fstream>
#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <filesystem>

#include "Game.h"
#include "Random.h"
#include "Components/Camera.h"
#include "Renderer/Renderer.h"
#include "Systems/ChessBoard.h"
#include "Systems/Events.h"

struct MainMenuModule
{
    explicit MainMenuModule(flecs::world &ecs);
};

struct PauseMenuModule
{
    explicit PauseMenuModule(flecs::world &ecs);
};

struct TestUIModule
{
    explicit TestUIModule(flecs::world &ecs);
};

MainMenuModule::MainMenuModule(flecs::world& ecs)
{
    ecs.system<const InputState>("MainMenu")
        .each([](const flecs::iter &it, size_t, const InputState &input)
        {
            const auto &ecs = it.world();
            if (ImGui::Begin("Era dos Fidalgos"))
            {
                if (ImGui::Button("Start"))
                {
                    void(ecs.entity<MainMenuModule>().disable());
                    void(ecs.entity<TestUIModule>().enable());
                }
                if (ImGui::Button("Quit") || input.WasEscapePressed)
                    ecs.quit();
            }
            ImGui::End();
        });
}

PauseMenuModule::PauseMenuModule(flecs::world& ecs)
{
    ecs.system<const InputState>("PauseMenu")
       .each([](const flecs::iter &it, size_t, const InputState &input)
       {
           const auto &ecs = it.world();
           if (ImGui::Begin("Paused"))
           {
               if (ImGui::Button("Resume") || input.WasEscapePressed)
               {
                   void(ecs.entity<PauseMenuModule>().disable());
                   void(ecs.entity<TestUIModule>().enable());
               }
               if (ImGui::Button("Quit"))
               {
                   void(ecs.entity<PauseMenuModule>().disable());
                   void(ecs.entity<MainMenuModule>().enable());
               }
           }
           ImGui::End();
       });
}

TestUIModule::TestUIModule(flecs::world& ecs)
{
    ecs.system("UpdateUI")
        .kind(flecs::OnUpdate)
        .run([](const flecs::iter &it)
        {
            const auto &ecs = it.world();
            if (auto input = ecs.try_get<InputState>(); input->WasEscapePressed)
            {
                void(ecs.entity<TestUIModule>().disable());
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

                static bool showEditor = false;
                ImGui::Checkbox("Show ImGUI Style Editor", &showEditor);
                if (showEditor) ImGui::ShowStyleEditor();
            }
            ImGui::End();
        });
}

void SetupImGuiStyle()
{
	// ayu-dark style by usrnatc from ImThemes
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 5.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 5.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 20.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 12.9f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 8.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 1.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.9019608f, 0.7058824f, 0.3137255f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.9019608f, 0.7058824f, 0.3137255f, 0.5019608f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.039215688f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.42745098f, 0.42745098f, 0.49803922f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07450981f, 0.09019608f, 0.12941177f, 1.0f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5019608f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.05882353f, 0.07450981f, 0.101960786f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5019608f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.043137256f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.9019608f, 0.7058824f, 0.3137255f, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.56078434f, 0.2509804f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30980393f, 0.31764707f, 0.3372549f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.13333334f, 0.4117647f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.13333334f, 0.4117647f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.07450981f, 0.09019608f, 0.12941177f, 1.0f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.06666667f, 0.101960786f, 0.14509805f, 0.9724f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13333334f, 0.25882354f, 0.42352942f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.13333334f, 0.4117647f, 0.54901963f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.2509804f, 0.25882354f, 0.2784314f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.039215688f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.039215688f, 0.05490196f, 0.078431375f, 1.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.06666667f, 0.10980392f, 0.16078432f, 1.0f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.24705882f, 0.69803923f, 1.0f, 1.0f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.9764706f, 0.25882354f, 0.25882354f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
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

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    SetupImGuiStyle();

    // Saves ini file in game config directory
    static char iniPathBuf[256] = {};
    const auto iniPath = std::filesystem::path(SDL_GetPrefPath("dcc_jogos", "EraDosFidalgos")) / "imgui.ini";
    strncpy(iniPathBuf, iniPath.c_str(), 255);
    io.IniFilename = iniPathBuf;

    // Setup Platform/Renderer backends
    if (ImGui_ImplSDL3_InitForOpenGL(window.sdlWindow, renderer.mContext) == false)
        return false;
    if (ImGui_ImplOpenGL3_Init("#version 330") == false)
        return false;

    // Init ECS
    RegisterSystems(ecs);

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

    void(ecs.import<MainMenuModule>());

    void(ecs.import<PauseMenuModule>().disable());

    void(ecs.import<TestUIModule>().disable());

    // Initialize CheckerBoard systems
    void(ecs.import<ChessBoardScene>()
        .child_of<TestUIModule>()
        .disable());

    // Initialize Event systems
    void(ecs.import<EventsSampleScene>()
        .child_of<TestUIModule>());

    void(ecs.import<CharactersModule>()
        .child_of<TestUIModule>());

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
