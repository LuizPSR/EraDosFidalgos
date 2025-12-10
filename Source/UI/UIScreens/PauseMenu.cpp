#include "PauseMenu.hpp"
#include "MainMenu.hpp"
#include <imgui.h>
#include <string>

#include "GameUIModule.hpp"

static bool HorizontalButton(const char* label, const ImVec2& size_arg = ImVec2(-FLT_MIN, 30.0f)) {
    return ImGui::Button(label, size_arg);
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


    // Obter informações da tela
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 screenSize = viewport->Size;
    ImVec2 screenPos = viewport->Pos;

    // Desenhar background
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    //ImTextureID bgTextureID = TextureToImTextureID(backgroundTexture);


        // Fallback: usar o gradiente azul medieval original
        ImU32 color1 = IM_COL32(10, 15, 40, 255);    // Azul muito escuro
        ImU32 color2 = IM_COL32(30, 45, 90, 255);    // Azul escuro
        ImU32 color3 = IM_COL32(60, 80, 120, 255);   // Azul médio

        // Gradiente vertical
        drawList->AddRectFilledMultiColor(
            screenPos,
            ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y * 0.5f),
            color1, color1, color2, color2
        );

        drawList->AddRectFilledMultiColor(
            ImVec2(screenPos.x, screenPos.y + screenSize.y * 0.5f),
            ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y),
            color2, color2, color3, color3
        );

        // Adicionar padrão de textura sutil (linhas diagonais)
        for (int i = -screenSize.y; i < screenSize.x + screenSize.y; i += 20) {
            drawList->AddLine(
                ImVec2(screenPos.x + i, screenPos.y),
                ImVec2(screenPos.x + i - screenSize.y, screenPos.y + screenSize.y),
                IM_COL32(255, 255, 255, 10)
            );
    }

    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2{0.5f, 0.5f});
    ImGui::SetNextWindowSizeConstraints(ImVec2{size.x * 0.6f, size.y * 0.2f}, ImVec2{FLT_MAX, FLT_MAX});

    if (ImGui::Begin("Paused", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize)) {
        if (HorizontalButton("Resume") || input.WasEscapePressed) {
            auto pauseMenuEntity = ecs.entity<PauseMenuModule>();
            auto testUIEntity = ecs.entity<GameUIModule>();

            if (pauseMenuEntity.is_valid()) pauseMenuEntity.disable();
            if (testUIEntity.is_valid()) testUIEntity.enable();
            tickSources.mTickTimer.start();
        }

        if (HorizontalButton("Quit")) {
            auto testUIEntity = ecs.entity<GameUIModule>();
            auto pauseMenuEntity = ecs.entity<PauseMenuModule>();
            auto mainMenuEntity = ecs.entity<MainMenuModule>();

            if (testUIEntity.is_valid()) testUIEntity.disable();
            if (pauseMenuEntity.is_valid()) pauseMenuEntity.disable();
            if (mainMenuEntity.is_valid()) mainMenuEntity.enable();
            ecs.add<GameEnded>();
        }
    }
    ImGui::End();
}