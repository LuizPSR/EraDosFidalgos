#include "MainMenu.hpp"
#include <imgui.h>
#include <string>
#include <SDL3/SDL.h>

#include "GameUIModule.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Texture.hpp"
#include "Systems/Characters.hpp"

// Variável global para a textura de background
static Texture* backgroundTexture = nullptr;
static bool textureLoaded = false;

// Função para carregar textura usando o sistema do Renderer
static void LoadBackgroundTexture(const flecs::world& ecs) {
    if (textureLoaded) return;
    textureLoaded = true;

    auto& renderer = ecs.get_mut<Renderer>();
    std::string backgroundPath = std::string(SDL_GetBasePath()) + "Assets/EDF II title.png";

    SDL_Log("Trying to load background: %s", backgroundPath.c_str());

    // Verificar se o arquivo existe primeiro
    SDL_IOStream* io = SDL_IOFromFile(backgroundPath.c_str(), "rb");
    if (!io) {
        SDL_Log("Background file not found: %s", backgroundPath.c_str());
        return;
    }
    SDL_CloseIO(io);

    // Usar o sistema de texturas do Renderer
    backgroundTexture = renderer.GetTexture(backgroundPath);

    if (backgroundTexture) {
        SDL_Log("Background texture loaded successfully via Renderer");
    } else {
        SDL_Log("Failed to load background texture via Renderer");
    }
}

// Função para converter Texture* para ImTextureID
static ImTextureID TextureToImTextureID(Texture* texture) {
    if (!texture) return (ImTextureID)0;
    return (ImTextureID)(intptr_t)texture->GetTextureID();
}

MainMenuModule::MainMenuModule(flecs::world& ecs) {
    ecs.system<GameTickSources, const InputState>("MainMenu")
        .each([](const flecs::iter &it, size_t, GameTickSources &tickSources, const InputState &input) {
            ShowMainMenu(it.world(), tickSources, input);
        });
}

void MainMenuModule::ShowMainMenu(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input) {
    // Carregar a textura na primeira execução
    if (!textureLoaded) {
        LoadBackgroundTexture(ecs);
    }

    // Obter informações da tela
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 screenSize = viewport->Size;
    ImVec2 screenPos = viewport->Pos;

    // Desenhar background
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImTextureID bgTextureID = TextureToImTextureID(backgroundTexture);

    if (bgTextureID != (ImTextureID)0) {
        // Desenhar a imagem de background cobrindo toda a tela
        drawList->AddImage(
            bgTextureID,
            screenPos,
            ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y),
            ImVec2(0, 0),
            ImVec2(1, 1)
        );

        // Adicionar overlay escuro para melhorar legibilidade
        drawList->AddRectFilled(
            screenPos,
            ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y),
            IM_COL32(0, 0, 0, 64)  // Overlay preto semi-transparente
        );
    } else {
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
    }

    // Resto do código do menu
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    const ImVec2 menuPosition = ImVec2(center.x, screenPos.y + screenSize.y * 0.80f);

    // Configurar a janela do menu
    ImGui::SetNextWindowPos(menuPosition, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(460, 200), ImGuiCond_Always);

    // Estilo para a janela do menu
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    if (ImGui::Begin("Era dos Fidalgos - Main Menu", nullptr,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoTitleBar)) {

        // Título centralizado
        ImVec2 titleSize = ImGui::CalcTextSize("ERA DOS FIDALGOS");
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSize.x) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        ImGui::Text("ERA DOS FIDALGOS");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // Botão START
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 250) * 0.5f);
        if (ImGui::Button("START", ImVec2(250, 38))) {
            auto mainMenuEntity = ecs.entity<MainMenuModule>();
            auto testUIEntity = ecs.entity<GameUIModule>();

            if (mainMenuEntity.is_valid()) mainMenuEntity.disable();
            if (testUIEntity.is_valid()) testUIEntity.enable();

            CreateKingdoms(ecs);

            tickSources.mTickTimer.start();
        }

        ImGui::Spacing();

        // Botão QUIT
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 250) * 0.5f);
        if (ImGui::Button("QUIT", ImVec2(250, 38)) || input.WasEscapePressed) {
            ecs.quit();
        }

        // Versão do jogo no rodapé
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 45);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("v0.1.0 Alpha").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 0.7f), "v0.1.0 Alpha");
    }
    ImGui::End();

    // Restaurar estilo
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}