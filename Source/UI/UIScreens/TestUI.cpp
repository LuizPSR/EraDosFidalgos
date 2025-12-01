
#include "TestUI.hpp"
#include <imgui.h>

#include "Components/Dynasty.hpp"
#include "Systems/ChessBoard.hpp"
#include "Systems/Characters.hpp"
#include "Systems/EstatePower.hpp" // Adicionar este include
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

// Função auxiliar para obter informações do jogador
static const Character* GetPlayerCharacter(const flecs::world& ecs) {
    flecs::entity playerEntity = ecs.entity<Player>();
    if (playerEntity.is_valid()) {
        return playerEntity.try_get<Character>();
    }
    return nullptr;
}

// Função auxiliar para obter o título do jogador
static flecs::entity GetPlayerTitle(const flecs::world& ecs) {
    flecs::entity playerEntity = ecs.entity<Player>();
    if (playerEntity.is_valid()) {
        return playerEntity.target<RulerOf>();
    }
    return flecs::entity::null();
}

// Função auxiliar para obter o poder dos estados
static const EstatePowers* GetEstatePowers(const flecs::world& ecs) {
    return ecs.try_get<EstatePowers>();
}

// Função auxiliar para formatar o ID do personagem em cores baseado no estado
static ImU32 GetPowerColor(int power) {
    if (power > 50) return IM_COL32(50, 200, 50, 255);     // Verde - muito favorável
    if (power > 20) return IM_COL32(150, 200, 50, 255);    // Verde-amarelo - favorável
    if (power > -20) return IM_COL32(200, 200, 50, 255);   // Amarelo - neutro
    if (power > -50) return IM_COL32(200, 150, 50, 255);   // Laranja - desfavorável
    return IM_COL32(200, 50, 50, 255);                     // Vermelho - muito desfavorável
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

        static bool showDemo = false;
        ImGui::Checkbox("Show ImGUI Demo", &showDemo);
        if (showDemo) ImGui::ShowDemoWindow();
    }
    ImGui::End();

    // 2. Janela popup do personagem no canto inferior esquerdo
    // Posicionar no canto inferior esquerdo
    ImVec2 windowPos = ImVec2(20, screenSize.y - 280); // Aumentei a altura para 280px
    ImVec2 windowSize = ImVec2(340, 260); // Aumentei o tamanho para 320x260

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    // Estilo para a janela do personagem
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.10f, 0.85f)); // Fundo azul escuro semi-transparente
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.6f, 0.2f, 0.8f));       // Borda dourada
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.8f, 1.0f));         // Texto creme
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 12));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 6));

    if (ImGui::Begin("Character Dashboard", nullptr,
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoScrollbar)) {

        // Obter informações do jogador
        const Character* playerChar = GetPlayerCharacter(ecs);
        flecs::entity playerTitleEntity = GetPlayerTitle(ecs);
        const Title* playerTitle = playerTitleEntity.is_valid() ? playerTitleEntity.try_get<Title>() : nullptr;
        const EstatePowers* estatePowers = GetEstatePowers(ecs);

        // Título interno com ícone de coroa
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Usar fonte padrão
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), " - Dashboard do Monarca - ");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();

        // Layout em duas colunas
        ImGui::Columns(2, "dashboardColumns", true);
        ImGui::SetColumnWidth(0, 120); // Coluna para a imagem

        // --- COLUNA 1: IMAGEM DO PERSONAGEM ---
        if (characterTextureLoaded) {
            ImVec2 imageSize = ImVec2(100, 120);
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImVec2 imagePos = ImVec2(
                cursorPos.x + (ImGui::GetColumnWidth() - imageSize.x) * 0.5f,
                cursorPos.y
            );

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImTextureID charTextureID = TextureToImTextureID(characterTexture);

            // Moldura dourada para a imagem
            drawList->AddRect(
                imagePos,
                ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                IM_COL32(180, 150, 50, 255),
                6.0f,
                0,
                2.0f
            );

            if (charTextureID != (ImTextureID)0) {
                drawList->AddImage(
                    charTextureID,
                    imagePos,
                    ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                    ImVec2(0, 0),
                    ImVec2(1, 1)
                );
            } else {
                // Placeholder
                drawList->AddRectFilled(
                    imagePos,
                    ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                    IM_COL32(40, 40, 60, 255)
                );
                drawList->AddText(
                    ImVec2(imagePos.x + 25, imagePos.y + 40),
                    IM_COL32(180, 180, 220, 255),
                    "Personagem"
                );
            }

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imageSize.y + 10);
        }

        // Nome do personagem centrado abaixo da imagem
        if (playerChar) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 180, 255));
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - ImGui::CalcTextSize(playerChar->mName.c_str()).x) * 0.5f);
            ImGui::Text("%s", playerChar->mName.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::NextColumn();

        // --- COLUNA 2: INFORMAÇÕES DETALHADAS ---
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(220, 220, 200, 255));

        // Informações do Personagem
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 200, 255, 255));
        ImGui::Text("Informações Pessoais");
        ImGui::PopStyleColor();
        
        if (playerChar) {
            // Idade
            uint64_t ageYears = playerChar->mAgeDays / 360;
            uint64_t ageMonths = (playerChar->mAgeDays / 30) % 12;
            ImGui::Text("Idade: %zu anos, %zu meses", ageYears, ageMonths);
            
            // Dinheiro
            //ImGui::Text("Tesouro: %.2f ouros", playerChar->FloatCost());
            
            // Dinastia
            flecs::entity dynastyTarget = ecs.entity<Player>().target<DynastyMember>();
            if (dynastyTarget.is_valid()) {
                auto* dynasty = dynastyTarget.try_get<Dynasty>();
                if (dynasty) {
                    ImGui::Text("Dinastia: %s", dynasty->name.c_str());
                }
            }
        }

        ImGui::Spacing();

        // Informações do Reino
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 200, 255, 255));
        ImGui::Text("Reino");
        ImGui::PopStyleColor();
        
        if (playerTitle) {
            ImGui::Text("Título: %s", playerTitle->name.c_str());
        } else {
            ImGui::Text("Título: Sem reino");
        }

        // Contar províncias do reino (simplificado)
        if (playerTitleEntity.is_valid()) {
            int provinceCount = 0;
            ecs.each<const Province>([&](flecs::entity p_entity, const Province&) {
                if (p_entity.has<InRealm>(playerTitleEntity)) {
                    provinceCount++;
                }
            });
            ImGui::Text("Províncias: %d", provinceCount);
        }

        ImGui::Spacing();

        // Poder dos Estados
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 200, 255, 255));
        ImGui::Text("Poder dos Estados");
        ImGui::PopStyleColor();
        
        if (estatePowers) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 4));
            
            // Nobreza
            ImU32 nobilityColor = GetPowerColor(estatePowers->mNobilityPower);
            ImGui::Text("  Nobreza:");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, nobilityColor);
            ImGui::Text(" %d", estatePowers->mNobilityPower);
            ImGui::PopStyleColor();
            
            // Clero
            ImU32 clergyColor = GetPowerColor(estatePowers->mClergyPower);
            ImGui::Text("  Clero:");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, clergyColor);
            ImGui::Text(" %d", estatePowers->mClergyPower);
            ImGui::PopStyleColor();
            
            // Povo
            ImU32 commonersColor = GetPowerColor(estatePowers->mCommonersPower);
            ImGui::Text("  Povo:");
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, commonersColor);
            ImGui::Text(" %d", estatePowers->mCommonersPower);
            ImGui::PopStyleColor();
            
            ImGui::PopStyleVar();
            
            // Barra de equilíbrio geral
            int totalPower = estatePowers->mNobilityPower + estatePowers->mClergyPower + estatePowers->mCommonersPower;
            float balance = std::abs(estatePowers->mNobilityPower) + std::abs(estatePowers->mClergyPower) + std::abs(estatePowers->mCommonersPower);
            balance = balance / 3.0f / 127.0f; // Normalizar
            
            ImGui::Spacing();
            ImGui::Text("Estabilidade:");
            ImGui::SameLine();
            
            ImU32 stabilityColor;
            if (balance < 0.25f) stabilityColor = IM_COL32(50, 200, 50, 255);
            else if (balance < 0.5f) stabilityColor = IM_COL32(200, 200, 50, 255);
            else if (balance < 0.75f) stabilityColor = IM_COL32(200, 150, 50, 255);
            else stabilityColor = IM_COL32(200, 50, 50, 255);
            
            ImGui::PushStyleColor(ImGuiCol_Text, stabilityColor);
            ImGui::Text(" %s", balance < 0.25f ? "Estável" : balance < 0.5f ? "Moderada" : balance < 0.75f ? "Tensa" : "Crítica");
            ImGui::PopStyleColor();
        } else {
            ImGui::Text("Poder dos estados: N/A");
        }

        ImGui::PopStyleColor(); // Para o estilo de texto geral

        ImGui::Columns(1); // Voltar para uma coluna
        ImGui::Spacing();

        // Rodapé com informações do jogo
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(150, 150, 180, 255));
        ImGui::Text("Era dos Fidalgos 0,2.0");
        ImGui::PopStyleColor();

    }
    ImGui::End();

    // Restaurar estilo
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(3);
}