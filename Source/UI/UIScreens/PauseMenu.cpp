#include "PauseMenu.hpp"
#include "MainMenu.hpp"
#include <imgui.h>
#include <string>

static bool HorizontalButton(const char* label, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0)) {
    return ImGui::Button(label, size_arg);
}

static flecs::entity GetEntityByName(const flecs::world& ecs, const std::string& name) {
    return ecs.lookup(name.c_str());
}

PauseMenuModule::PauseMenuModule(flecs::world& ecs) {
    ecs.system<GameTickSources, const InputState>("PauseMenu")
       .each([](const flecs::iter &it, size_t, GameTickSources &tickSources, const InputState &input) {
           ShowPauseMenu(it.world(), tickSources, input);
       });
}

void PauseMenuModule::ShowPauseMenu(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input) {
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const ImVec2 size = ImGui::GetMainViewport()->Size;

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    ImGui::SetNextWindowSizeConstraints(ImVec2{size.x * 0.6f, size.y * 0.2f}, ImVec2{FLT_MAX, FLT_MAX});

    if (ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (HorizontalButton("Resume") || input.WasEscapePressed) {
            auto pauseMenuEntity = GetEntityByName(ecs, "PauseMenuModule");
            auto testUIEntity = GetEntityByName(ecs, "TestUIModule");

            if (pauseMenuEntity.is_valid()) pauseMenuEntity.disable();
            if (testUIEntity.is_valid()) testUIEntity.enable();
            tickSources.mTickTimer.start();
        }

        if (HorizontalButton("Quit")) {
            auto testUIEntity = GetEntityByName(ecs, "TestUIModule");
            auto pauseMenuEntity = GetEntityByName(ecs, "PauseMenuModule");
            auto mainMenuEntity = ecs.entity<MainMenuModule>();

            if (testUIEntity.is_valid()) testUIEntity.disable();
            if (pauseMenuEntity.is_valid()) pauseMenuEntity.disable();
            if (mainMenuEntity.is_valid()) mainMenuEntity.enable();
        }
    }
    ImGui::End();
}