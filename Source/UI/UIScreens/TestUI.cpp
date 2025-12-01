#include "TestUI.hpp"
#include <imgui.h>
#include "Systems/ChessBoard.hpp"
#include "Systems/Characters.hpp"

static flecs::entity GetEntityByName(const flecs::world& ecs, const std::string& name) {
    return ecs.lookup(name.c_str());
}

TestUIModule::TestUIModule(flecs::world& ecs) {
    const flecs::entity tickTimer = ecs.get<GameTickSources>().mTickTimer;

    ecs.system<GameTickSources>("UpdateUI")
        .tick_source(tickTimer)
        .kind(flecs::OnUpdate)
        .each([](const flecs::iter &it, size_t, GameTickSources &tickSources) {
            ShowTestUI(it.world(), tickSources);
        });
}

void TestUIModule::ShowTestUI(const flecs::world& ecs, GameTickSources& tickSources) {
    auto input = ecs.try_get<InputState>();
    if (input && input->WasEscapePressed) {
        tickSources.mTickTimer.stop();
        auto pauseMenuEntity = GetEntityByName(ecs, "PauseMenuModule");
        auto testUIEntity = GetEntityByName(ecs, "TestUIModule");

        if (pauseMenuEntity.is_valid()) pauseMenuEntity.enable();
        if (testUIEntity.is_valid()) testUIEntity.disable();
    }

    if (ImGui::Begin("Menu")) {
        ImGui::Text("Application Average %.3f ms/frame (%.1f FPS)",
                   1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Separator();

        if (ImGui::Button("Click Me")) {
            SDL_Log("I've been clicked!");
        }

        glm::vec2 mPos;
        SDL_GetRelativeMouseState(&mPos.x, &mPos.y);
        ImGui::Text("Mouse Relative: %.1f %.1f", mPos.x, mPos.y);

        static bool chessboardActive = false;
        if (ImGui::Checkbox("Chessboard", &chessboardActive)) {
            auto chessboardEntity = ecs.entity<ChessBoardScene>();
            if (!chessboardActive) {
                ChessBoardScene::Stop(chessboardEntity.disable());
            } else {
                ChessBoardScene::Start(chessboardEntity.enable());
            }
        }

        static bool showDemo = false;
        ImGui::Checkbox("Show ImGUI Demo", &showDemo);
        if (showDemo) ImGui::ShowDemoWindow();
    }
    ImGui::End();
}