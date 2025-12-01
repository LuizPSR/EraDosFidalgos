#include "MainMenu.hpp"
#include <imgui.h>
#include <string>
#include <SDL3/SDL.h>

static bool HorizontalButton(const char* label, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0)) {
    return ImGui::Button(label, size_arg);
}

static flecs::entity GetEntityByName(const flecs::world& ecs, const std::string& name) {
    return ecs.lookup(name.c_str());
}

MainMenuModule::MainMenuModule(flecs::world& ecs) {
    ecs.system<GameTickSources, const InputState>("MainMenu")
        .each([](const flecs::iter &it, size_t, GameTickSources &tickSources, const InputState &input) {
            ShowMainMenu(it.world(), tickSources, input);
        });
}

void MainMenuModule::ShowMainMenu(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input) {
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const ImVec2 size = ImGui::GetMainViewport()->Size;

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    ImGui::SetNextWindowSizeConstraints(ImVec2{size.x * 0.6f, size.y * 0.2f}, ImVec2{FLT_MAX, FLT_MAX});

    if (ImGui::Begin("Era dos Fidalgos", nullptr,
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground)) {

        if (HorizontalButton("Start")) {
            auto mainMenuEntity = ecs.entity<MainMenuModule>();
            auto testUIEntity = GetEntityByName(ecs, "TestUIModule");

            if (mainMenuEntity.is_valid()) mainMenuEntity.disable();
            if (testUIEntity.is_valid()) testUIEntity.enable();
            tickSources.mTickTimer.start();
        }

        if (HorizontalButton("Quit") || input.WasEscapePressed) {
            ecs.quit();
        }
        }
    ImGui::End();
}