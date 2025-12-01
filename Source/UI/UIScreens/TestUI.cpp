#include "TestUI.hpp"
#include <imgui.h>
#include "Systems/ChessBoard.hpp"
#include "Systems/Characters.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Texture.hpp"

static flecs::entity GetEntityByName(const flecs::world& ecs, const std::string& name) {
    return ecs.lookup(name.c_str());
}

// Variáveis para o popup do personagem
static Texture* characterTexture = nullptr;
static bool characterTextureLoaded = false;

// Função para carregar a textura do personagem
static void LoadCharacterTexture(const flecs::world& ecs) {
    if (characterTextureLoaded) return;
    characterTextureLoaded = true;

    auto& renderer = ecs.get_mut<Renderer>();
    std::string characterPath = std::string(SDL_GetBasePath()) + "Assets/Personagem/Modelos Personagem/default.png";

    SDL_Log("Trying to load character sprite: %s", characterPath.c_str());

    // Verificar se o arquivo existe primeiro
    SDL_IOStream* io = SDL_IOFromFile(characterPath.c_str(), "rb");
    if (!io) {
        SDL_Log("Character sprite file not found: %s", characterPath.c_str());
        return;
    }
    SDL_CloseIO(io);

    // Usar o sistema de texturas do Renderer
    characterTexture = renderer.GetTexture(characterPath);

    if (characterTexture) {
        SDL_Log("Character sprite loaded successfully");
    } else {
        SDL_Log("Failed to load character sprite");
    }
}

// Função para converter Texture* para ImTextureID
static ImTextureID TextureToImTextureID(Texture* texture) {
    if (!texture) return (ImTextureID)0;
    return (ImTextureID)(intptr_t)texture->GetTextureID();
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

    // Obter informações da tela
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 screenSize = viewport->Size;
    ImVec2 screenPos = viewport->Pos;

    // TODO: replace this with something that won't draw over map
    // // Desenhar fundo gradiente usando ImDrawList
    // ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    //
    // // Gradiente azul medieval
    // ImU32 color1 = IM_COL32(10, 15, 40, 255);    // Azul muito escuro
    // ImU32 color2 = IM_COL32(30, 45, 90, 255);    // Azul escuro
    // ImU32 color3 = IM_COL32(60, 80, 120, 255);   // Azul médio
    //
    // // Gradiente vertical
    // drawList->AddRectFilledMultiColor(
    //     screenPos,
    //     ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y * 0.5f),
    //     color1, color1, color2, color2
    // );
    //
    // drawList->AddRectFilledMultiColor(
    //     ImVec2(screenPos.x, screenPos.y + screenSize.y * 0.5f),
    //     ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y),
    //     color2, color2, color3, color3
    // );
    //
    // // Adicionar padrão de textura sutil (linhas diagonais)
    // for (int i = -screenSize.y; i < screenSize.x + screenSize.y; i += 20) {
    //     drawList->AddLine(
    //         ImVec2(screenPos.x + i, screenPos.y),
    //         ImVec2(screenPos.x + i - screenSize.y, screenPos.y + screenSize.y),
    //         IM_COL32(255, 255, 255, 10)
    //     );
    // }

    // Carregar a textura do personagem na primeira execução
    if (!characterTextureLoaded) {
        LoadCharacterTexture(ecs);
    }

    // 1. Primeiro a janela principal do menu
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

    // 2. Agora a janela popup do personagem no canto inferior esquerdo
    // Posicionar no canto inferior esquerdo
    ImVec2 windowPos = ImVec2(20, screenSize.y - 220); // 20px da esquerda, 220px do fundo
    ImVec2 windowSize = ImVec2(150, 210); // Tamanho fixo

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    // Estilo para a janela do personagem (transparente, sem bordas)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Fundo semi-transparente
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.6f, 0.2f, 0.5f));   // Borda dourada sutil
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));

    if (ImGui::Begin("Character Preview", nullptr,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollbar)) {

        // Título interno
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Personagem");
        ImGui::Separator();
        ImGui::Spacing();

        // Área para a imagem do personagem
        ImVec2 imageSize = ImVec2(100, 120); // Tamanho fixo para a imagem
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImVec2 windowCenter = ImVec2(
            cursorPos.x + (ImGui::GetWindowWidth() - imageSize.x) * 0.5f,
            cursorPos.y
        );

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImTextureID charTextureID = TextureToImTextureID(characterTexture);

        if (charTextureID != (ImTextureID)0) {
            // Desenhar a imagem do personagem centralizada
            drawList->AddImage(
                charTextureID,
                windowCenter,
                ImVec2(windowCenter.x + imageSize.x, windowCenter.y + imageSize.y),
                ImVec2(0, 0),
                ImVec2(1, 1)
            );
        } else {
            // Fallback: desenhar um placeholder
            drawList->AddRectFilled(
                windowCenter,
                ImVec2(windowCenter.x + imageSize.x, windowCenter.y + imageSize.y),
                IM_COL32(50, 50, 80, 255)
            );

            // Desenhar um ícone de personagem simples
            drawList->AddCircle(
                ImVec2(windowCenter.x + imageSize.x * 0.5f, windowCenter.y + imageSize.y * 0.4f),
                imageSize.x * 0.2f,
                IM_COL32(200, 200, 255, 255),
                0, 2.0f
            );

            drawList->AddRectFilled(
                ImVec2(windowCenter.x + imageSize.x * 0.4f, windowCenter.y + imageSize.y * 0.6f),
                ImVec2(windowCenter.x + imageSize.x * 0.6f, windowCenter.y + imageSize.y * 0.8f),
                IM_COL32(200, 200, 255, 255)
            );

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageSize.y);
        }

        ImGui::Spacing();

    }
    ImGui::End();

    // Restaurar estilo
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}
