#include "GameOver.hpp"
#include "UIScreens/MainMenu.hpp"
#include <imgui.h>
#include <string>
#include <SDL3/SDL.h>
#include "Renderer/Renderer.hpp"
#include "Renderer/Texture.hpp"
#include "Systems/Characters.hpp"
#include "UI/UIScreens/UICommon.hpp"
#include "UIScreens/GameUIModule.hpp"
#include "UIScreens/PauseMenu.hpp"

// Variáveis estáticas para armazenar as informações do game over
namespace {
    Texture* gameOverTexture = nullptr;
    bool gameOverTextureLoaded = false;

    std::string gameOverCause = "";
    std::string playerName = "";
    uint64_t playerAgeYears = 0;
    std::string playerTitle = "";
    std::string playerDynasty = "";
    float playerGold = 0.0f;

    // Flag para controlar se estamos na tela de game over
    bool isGameOverActive = false;
}

// Função para carregar textura usando o sistema do Renderer
static void LoadGameOverTexture(const flecs::world& ecs) {
    if (gameOverTextureLoaded) return;
    gameOverTextureLoaded = true;

    auto& renderer = ecs.get_mut<Renderer>();
    std::string texturePath = std::string(SDL_GetBasePath()) + "Assets/GameOverBackground.png";

    SDL_Log("Trying to load game over background: %s", texturePath.c_str());

    // Verificar se o arquivo existe primeiro
    SDL_IOStream* io = SDL_IOFromFile(texturePath.c_str(), "rb");
    if (!io) {
        SDL_Log("Game over background file not found: %s", texturePath.c_str());
        return;
    }
    SDL_CloseIO(io);

    // Usar o sistema de texturas do Renderer
    gameOverTexture = renderer.GetTexture(texturePath);

    if (gameOverTexture) {
        SDL_Log("Game over background texture loaded successfully via Renderer");
    } else {
        SDL_Log("Failed to load game over background texture via Renderer");
    }
}

// Função para converter Texture* para ImTextureID
static ImTextureID TextureToImTextureID(Texture* texture) {
    if (!texture) return (ImTextureID)0;
    return (ImTextureID)(intptr_t)texture->GetTextureID();
}

// Implementação das funções públicas
void GameOverModule::SetGameOverInfo(const std::string& cause,
                                     const std::string& pName,
                                     uint64_t pAgeYears,
                                     const std::string& pTitle,
                                     const std::string& pDynasty,
                                     float pGold) {
    gameOverCause = cause;
    playerName = pName;
    playerAgeYears = pAgeYears;
    playerTitle = pTitle;
    playerDynasty = pDynasty;
    playerGold = pGold;
    isGameOverActive = true;
}

std::string GameOverModule::FormatGameOverCause(const EstatePowers* powers) {
    std::string cause = "O reino colapsou devido à má gestão.\n\n";

    if (powers->mCommonersPower <= -100)
        cause += "A plebe, irada pela vida precária, invade o palácio.\n";
    if (powers->mCommonersPower >= 100)
        cause += "A burguesia resolve livrar-se da família real e instaurar uma república.\n";
    if (powers->mClergyPower <= -100)
        cause += "Devido à perseguição extrema ao clero, sofreu um julgamento divino.\n";
    if (powers->mClergyPower >= 100)
        cause += "Espiritualistas tomam conta do clero e da nação, proclamando que a matéria é má e que ninguém deve obedecer a rei algum.\n";
    if (powers->mNobilityPower <= -100)
        cause += "Os nobres vassalos não mais respeitam os seus contratos, já que você não respeitou sua parte, e o reino já não existe.\n";
    if (powers->mNobilityPower >= 100)
        cause += "O poder que rege no reino já não é mais o seu, o povo respeita e admira mais a nobreza local, e eles decidem dividir o poder e eleger um novo monarca entre si.\n";

    return cause;
}

// Função para resetar completamente o estado do game over
static void ResetGameOverState() {
    gameOverCause = "";
    playerName = "";
    playerAgeYears = 0;
    playerTitle = "";
    playerDynasty = "";
    playerGold = 0.0f;
    isGameOverActive = false;
}

GameOverModule::GameOverModule(flecs::world& ecs) {
    ecs.system<GameTickSources, const InputState>("GameOverScreen")
        .each([](const flecs::iter &it, size_t, GameTickSources &tickSources, const InputState &input) {
            ShowGameOverScreen(it.world(), tickSources, input);
        });
}

void GameOverModule::ShowGameOverScreen(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input) {
    // Só mostrar se realmente estamos em estado de game over
    if (!isGameOverActive) return;

    ShowGameOverScreen(ecs, tickSources, input, gameOverCause, nullptr);
}

void GameOverModule::ShowGameOverScreen(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input,
                                        const std::string& cause, const Character* playerChar) {
    // Só mostrar se realmente estamos em estado de game over
    if (!isGameOverActive) return;

    // Carregar a textura na primeira execução
    if (!gameOverTextureLoaded) {
        LoadGameOverTexture(ecs);
    }

    // Obter informações da tela
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 screenSize = viewport->Size;
    ImVec2 screenPos = viewport->Pos;

    // Desenhar background
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    ImTextureID bgTextureID = TextureToImTextureID(gameOverTexture);

    if (bgTextureID != (ImTextureID)0) {
        // Desenhar a imagem de background cobrindo toda a tela
        drawList->AddImage(
            bgTextureID,
            screenPos,
            ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y),
            ImVec2(0, 0),
            ImVec2(1, 1)
        );

        // Adicionar overlay vermelho/escuro para tema de game over
        drawList->AddRectFilled(
            screenPos,
            ImVec2(screenPos.x + screenSize.x, screenPos.y + screenSize.y),
            IM_COL32(40, 0, 0, 192)  // Overlay vermelho escuro semi-transparente
        );
    } else {
        // Fallback: usar gradiente vermelho escuro para game over
        ImU32 color1 = IM_COL32(30, 0, 0, 255);    // Vermelho muito escuro
        ImU32 color2 = IM_COL32(60, 0, 10, 255);   // Vermelho escuro
        ImU32 color3 = IM_COL32(90, 0, 20, 255);   // Vermelho médio escuro

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

        // Adicionar padrão de textura sutil (cruzes ou símbolos de morte)
        for (int i = 0; i < screenSize.x; i += 60) {
            for (int j = 0; j < screenSize.y; j += 60) {
                drawList->AddText(
                    ImVec2(screenPos.x + i, screenPos.y + j),
                    IM_COL32(100, 0, 0, 30),
                    "†"
                );
            }
        }
    }

    // Configurar a janela principal do game over - AUMENTEI O TAMANHO
    const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(ImVec2(center.x, screenPos.y + screenSize.y * 0.25f),
                           ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(screenSize.x * 0.8f, screenSize.y * 0.7f), ImGuiCond_Always); // Aumentei para 80% x 70%

    // Estilo para a janela do game over
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 15.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40)); // Aumentei o padding
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 20)); // Aumentei o espaçamento

    if (ImGui::Begin("Game Over", nullptr,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoTitleBar)) {

        // Título GAME OVER
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.1f, 0.1f, 1.0f));

        // Calcular tamanho e centralizar
        const char* titleText = "GAME OVER";
        ImVec2 titleSize = ImGui::CalcTextSize(titleText);

        // Adicionar espaço extra acima do título
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSize.x) * 0.5f);

        // Texto maior (usando tamanho de fonte aumentado temporariamente)
        ImGui::SetWindowFontScale(1.5f);
        ImGui::Text("%s", titleText);
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        // Causa do game over
        std::string displayCause = cause.empty() ? gameOverCause : cause;
        if (!displayCause.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.8f, 1.0f));
            ImGui::TextWrapped("%s", displayCause.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        // Seções com informações do jogador (se disponível)
        ImGui::BeginChild("PlayerInfo", ImVec2(0, ImGui::GetContentRegionAvail().y - 140), true); // Aumentei a altura

        if (!playerName.empty()) {
            // Informações pessoais com fonte ligeiramente maior
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.6f, 1.0f));
            ImGui::SetWindowFontScale(1.1f);
            ImGui::Text("ESTATÍSTICAS DO REINADO");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Spacing();

            // Nome e título - AUMENTEI AS COLUNAS
            ImGui::Columns(2, "statsColumns", false);
            ImGui::SetColumnWidth(0, 180); // Aumentei a largura da primeira coluna

            ImGui::Text("Monarca:");
            ImGui::NextColumn();
            ImGui::Text("%s", playerName.c_str());
            ImGui::NextColumn();

            ImGui::Text("Idade:");
            ImGui::NextColumn();
            ImGui::Text("%zu anos", playerAgeYears);
            ImGui::NextColumn();

            if (!playerTitle.empty()) {
                ImGui::Text("Título:");
                ImGui::NextColumn();
                ImGui::Text("%s", playerTitle.c_str());
                ImGui::NextColumn();
            }

            if (!playerDynasty.empty()) {
                ImGui::Text("Dinastia:");
                ImGui::NextColumn();
                ImGui::Text("%s", playerDynasty.c_str());
                ImGui::NextColumn();
            }

            ImGui::Text("Tesouro:");
            ImGui::NextColumn();
            ImGui::Text("%.2f ouros", playerGold);
            ImGui::NextColumn();

            ImGui::Columns(1);

            // Linha separadora
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Resumo adicional
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
            ImGui::TextWrapped("O seu reinado chegou ao fim. A história registrará suas conquistas e falhas.");
            ImGui::PopStyleColor();
        } else {
            // Mensagem padrão quando não há informações do jogador
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
            ImGui::TextWrapped("Seu reinado chegou ao fim. A história julgará suas ações e decisões.");
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();

        // Botões na parte inferior - AUMENTEI OS BOTÕES
        ImGui::Spacing();
        ImGui::Spacing();

        float buttonWidth = 250; // Aumentei a largura
        float buttonHeight = 55; // Aumentei a altura
        float totalWidth = buttonWidth * 2 + 30; // Dois botões com espaçamento maior
        float startX = (ImGui::GetWindowWidth() - totalWidth) * 0.5f;

        ImGui::SetCursorPosX(startX);

        // Botão RESTART
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        if (ImGui::Button("NOVO REINO", ImVec2(buttonWidth, buttonHeight))) {
            auto gameOverEntity = ecs.entity<GameOverModule>();
            auto mainMenuEntity = ecs.entity<MainMenuModule>();
            auto testUIEntity = ecs.entity<GameUIModule>();
            auto pauseMenuEntity = ecs.entity<PauseMenuModule>();

            // Desativar todas as UIs de jogo
            if (gameOverEntity.is_valid()) gameOverEntity.disable();
            if (testUIEntity.is_valid()) testUIEntity.disable();
            if (pauseMenuEntity.is_valid()) pauseMenuEntity.disable();

            // Ativar menu principal
            if (mainMenuEntity.is_valid()) mainMenuEntity.enable();

            // Resetar completamente o estado do game over
            ResetGameOverState();

            // Parar todos os timers
            tickSources.mTickTimer.stop();
            tickSources.mDayTimer.stop();
            tickSources.mWeekTimer.stop();
            tickSources.mMonthTimer.stop();
            tickSources.mYearTimer.stop();

            // Remover tag de pausa do jogo se existir
            if (ecs.lookup("GameOver").is_valid()) {
                ecs.lookup("GameOver").remove<PausesGame>();
            }
        }

        ImGui::PopStyleColor(4);

        ImGui::SameLine();

        // Botão QUIT
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

        if (ImGui::Button("SAIR DO JOGO", ImVec2(buttonWidth, buttonHeight)) || input.WasEscapePressed) {
            ecs.quit();
        }

        ImGui::PopStyleColor(4);

        // Versão do jogo no rodapé
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - ImGui::CalcTextSize("Era dos Fidalgos - Game Over").x) * 0.5f);
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 0.7f), "Era dos Fidalgos - Game Over");
    }
    ImGui::End();

    // Restaurar estilo
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(3);
}