#include "GameUIModule.hpp"
#include <imgui.h>

#include "PauseMenu.hpp"
#include "Components/Dynasty.hpp"
#include "Systems/GameBoard.hpp"
#include "Systems/Characters.hpp"
#include "Systems/EstatePower.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Texture.hpp"
#include "Systems/MapGenerator.hpp"
#include "Random.hpp"  // Adicionar este include para usar Random

// Variáveis para o popup do personagem
static Texture* characterTexture = nullptr;
static bool characterTextureLoaded = false;
static int selectedCharacterIndex = -1;  // Índice do personagem selecionado

// Função para escolher aleatoriamente um arquivo de personagem
static std::string GetRandomCharacterFilePath() {
    // Lista de possíveis arquivos de personagem
    const int numCharacters = 5;  // p1.png a p5.png
    int randomIndex = Random::GetIntRange(1, numCharacters);  // 1-5

    selectedCharacterIndex = randomIndex;  // Armazenar o índice selecionado

    // Retornar o caminho do arquivo
    return std::string(SDL_GetBasePath()) +
           "Assets/Personagem/Modelos Personagem/p" +
           std::to_string(randomIndex) + ".png";
}

// Função para obter o título baseado no índice do personagem
static std::string GetCharacterTitle() {
    if (selectedCharacterIndex == -1) {
        return "Monarca";
    }

    // Títulos diferentes baseados no personagem selecionado
    switch (selectedCharacterIndex) {
        case 1: return "Rei Valoroso";
        case 2: return "Sábio Governante";
        case 3: return "Estrategista Real";
        case 4: return "Protetor do Reino";
        case 5: return "Imperador Justo";
        default: return "Monarca";
    }
}

// Função para obter a descrição baseada no índice do personagem
static std::string GetCharacterDescription() {
    if (selectedCharacterIndex == -1) {
        return "Líder do reino";
    }

    switch (selectedCharacterIndex) {
        case 1: return "Corajoso líder de batalhas";
        case 2: return "Sábio administrador do reino";
        case 3: return "Estrategista militar experiente";
        case 4: return "Protetor do povo e das terras";
        case 5: return "Justo e respeitado imperador";
        default: return "Líder do reino";
    }
}

// Função para carregar a textura do personagem
static void LoadCharacterTexture(const flecs::world& ecs) {
    if (characterTextureLoaded) return;
    characterTextureLoaded = true;

    auto& renderer = ecs.get_mut<Renderer>();

    // Tentar carregar um personagem aleatório
    std::string characterPath = GetRandomCharacterFilePath();
    bool fileFound = false;

    // Verificar se o arquivo existe primeiro
    SDL_IOStream* io = SDL_IOFromFile(characterPath.c_str(), "rb");
    if (io) {
        SDL_CloseIO(io);
        fileFound = true;
        SDL_Log("Selected character sprite: p%d.png", selectedCharacterIndex);
    } else {
        SDL_Log("Random character file not found: %s", characterPath.c_str());

        // Tentar fallback para default.png
        characterPath = std::string(SDL_GetBasePath()) + "Assets/Personagem/Modelos Personagem/default.png";
        io = SDL_IOFromFile(characterPath.c_str(), "rb");
        if (io) {
            SDL_CloseIO(io);
            fileFound = true;
            selectedCharacterIndex = 0;  // Usar 0 para default
            SDL_Log("Using default character sprite as fallback");
        } else {
            SDL_Log("Default character sprite also not found");
            return;
        }
    }

    // Usar o sistema de texturas do Renderer
    characterTexture = renderer.GetTexture(characterPath);

    if (characterTexture) {
        SDL_Log("Character sprite p%d.png loaded successfully",
                selectedCharacterIndex > 0 ? selectedCharacterIndex : 0);
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

GameUIModule::GameUIModule(flecs::world& ecs) {
    const flecs::entity tickTimer = ecs.get<GameTickSources>().mTickTimer;

    ecs.system<GameTickSources>("UpdateUI")
        .tick_source(tickTimer)
        .kind(flecs::OnUpdate)
        .each([](const flecs::iter &it, size_t, GameTickSources &tickSources) {
            ShowTestUI(it.world(), tickSources);
        });
}

void GameUIModule::ShowTestUI(const flecs::world& ecs, GameTickSources& tickSources) {
    auto input = ecs.try_get<InputState>();
    if (input && input->WasEscapePressed) {
        tickSources.mTickTimer.stop();
        auto pauseMenuEntity = ecs.entity<PauseMenuModule>();
        auto testUIEntity = ecs.entity<GameUIModule>();

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

        const auto &cameraTarget = ecs.get<Camera>().mTarget;
        ImGui::Text("Camera target: %.2f %.2f", cameraTarget.x, cameraTarget.y);
        ImGui::Separator();

        glm::vec2 mPos;
        SDL_GetRelativeMouseState(&mPos.x, &mPos.y);
        ImGui::Text("Mouse Relative: %.1f %.1f", mPos.x, mPos.y);
        ImGui::Separator();

        auto &powers = ecs.get_mut<EstatePowers>();
        int32_t commonersPower = powers.mCommonersPower;
        ImGui::InputInt("Plebe", &commonersPower, 1, 10);
        powers.mCommonersPower = commonersPower;

        // Botão para recarregar personagem aleatório
        ImGui::Separator();
        if (ImGui::Button("Mudar Personagem")) {
            // Recarregar com novo personagem aleatório
            if (characterTexture) {
                // Liberar textura antiga se necessário
                // (o sistema do Renderer pode gerenciar isso automaticamente)
            }
            characterTextureLoaded = false;
            selectedCharacterIndex = -1;
            LoadCharacterTexture(ecs);
            SDL_Log("Personagem alterado para p%d.png", selectedCharacterIndex);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Clique para selecionar um novo personagem aleatório");
        }
    }
    ImGui::End();

    // 2. Janela popup do personagem no canto inferior esquerdo
    ImVec2 windowPos = ImVec2(20, screenSize.y - 320); // Aumentei a altura
    ImVec2 windowSize = ImVec2(360, 300); // Aumentei o tamanho

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Always);

    // Estilo para a janela do personagem
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.10f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8f, 0.6f, 0.2f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.8f, 1.0f));
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

        // Título interno com ícone de coroa e título do personagem
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        std::string dashboardTitle = " - " + GetCharacterTitle() + " - ";
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%s", dashboardTitle.c_str());
        ImGui::PopFont();

        // Descrição do personagem (pequena)
        if (selectedCharacterIndex > 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 200, 220, 200));
            ImGui::SetWindowFontScale(0.8f);
            ImGui::Text("%s", GetCharacterDescription().c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
        }

        ImGui::Separator();
        ImGui::Spacing();

        // Layout em duas colunas
        ImGui::Columns(2, "dashboardColumns", true);
        ImGui::SetColumnWidth(0, 130); // Aumentei a coluna da imagem

        // --- COLUNA 1: IMAGEM DO PERSONAGEM ---
        if (characterTextureLoaded) {
            ImVec2 imageSize = ImVec2(110, 130); // Aumentei o tamanho da imagem
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            ImVec2 imagePos = ImVec2(
                cursorPos.x + (ImGui::GetColumnWidth() - imageSize.x) * 0.5f,
                cursorPos.y
            );

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImTextureID charTextureID = TextureToImTextureID(characterTexture);

            // Moldura dourada para a imagem com efeito de brilho
            drawList->AddRect(
                imagePos,
                ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                IM_COL32(180, 150, 50, 255),
                8.0f,  // Aumentei o arredondamento
                0,
                2.5f   // Aumentei a espessura
            );

            // Efeito de brilho interno
            drawList->AddRect(
                ImVec2(imagePos.x + 1, imagePos.y + 1),
                ImVec2(imagePos.x + imageSize.x - 1, imagePos.y + imageSize.y - 1),
                IM_COL32(220, 190, 80, 100),
                7.0f,
                0,
                1.0f
            );

            if (charTextureID != (ImTextureID)0) {
                drawList->AddImage(
                    charTextureID,
                    imagePos,
                    ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                    ImVec2(0, 0),
                    ImVec2(1, 1),
                    IM_COL32(255, 255, 255, 255) // Cor branca total
                );

                // Indicador de qual personagem está sendo usado
                if (selectedCharacterIndex > 0) {
                    std::string charIndicator = "P" + std::to_string(selectedCharacterIndex);
                    ImVec2 textSize = ImGui::CalcTextSize(charIndicator.c_str());
                    ImVec2 textPos = ImVec2(
                        imagePos.x + imageSize.x - textSize.x - 5,
                        imagePos.y + 5
                    );

                    // Fundo para o indicador
                    drawList->AddRectFilled(
                        ImVec2(textPos.x - 3, textPos.y - 2),
                        ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 2),
                        IM_COL32(0, 0, 0, 150)
                    );

                    // Texto do indicador
                    drawList->AddText(
                        textPos,
                        IM_COL32(255, 220, 100, 255),
                        charIndicator.c_str()
                    );
                }
            } else {
                // Placeholder com design melhor
                drawList->AddRectFilled(
                    imagePos,
                    ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                    IM_COL32(40, 40, 60, 255)
                );
                drawList->AddRect(
                    imagePos,
                    ImVec2(imagePos.x + imageSize.x, imagePos.y + imageSize.y),
                    IM_COL32(80, 80, 120, 255),
                    8.0f
                );
                drawList->AddText(
                    ImVec2(imagePos.x + 20, imagePos.y + 50),
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

            // Dinastia
            flecs::entity dynastyTarget = ecs.entity<Player>().target<DynastyMember>();
            if (dynastyTarget.is_valid()) {
                auto* dynasty = dynastyTarget.try_get<Dynasty>();
                if (dynasty) {
                    ImGui::Text("Dinastia: %s", dynasty->name.c_str());
                }
            }

            // Título do personagem (se selecionado)
            if (selectedCharacterIndex > 0) {
                ImGui::Text("Título: %s", GetCharacterTitle().c_str());
            }
        }

        ImGui::Spacing();

        // Informações do Reino
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 200, 255, 255));
        ImGui::Text("Reino");
        ImGui::PopStyleColor();

        if (playerTitle) {
            ImGui::Text("Domínio: %s", playerTitle->name.c_str());
        } else {
            ImGui::Text("Domínio: Sem reino");
        }

        // Contar províncias do reino
        if (playerTitleEntity.is_valid()) {
            int provinceCount = 0;
            ecs.each<const Province>([&](flecs::entity p_entity, const Province&) {
                if (p_entity.has<InRealm>(playerTitleEntity)) {
                    provinceCount++;
                }
            });
            ImGui::Text("Províncias: %d", provinceCount);
            ImGui::Text("Ouro: %.2f", playerChar->MoneyFloat());
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
            float balance = std::abs(estatePowers->mNobilityPower) +
                           std::abs(estatePowers->mClergyPower) +
                           std::abs(estatePowers->mCommonersPower);
            balance = balance / 3.0f / 127.0f; // Normalizar

            ImGui::Spacing();
            ImGui::Text("Estabilidade:");
            ImGui::SameLine();

            ImU32 stabilityColor;
            std::string stabilityText;
            if (balance < 0.25f) {
                stabilityColor = IM_COL32(50, 200, 50, 255);
                stabilityText = "Estável";
            } else if (balance < 0.5f) {
                stabilityColor = IM_COL32(200, 200, 50, 255);
                stabilityText = "Moderada";
            } else if (balance < 0.75f) {
                stabilityColor = IM_COL32(200, 150, 50, 255);
                stabilityText = "Tensa";
            } else {
                stabilityColor = IM_COL32(200, 50, 50, 255);
                stabilityText = "Crítica";
            }

            ImGui::PushStyleColor(ImGuiCol_Text, stabilityColor);
            ImGui::Text(" %s", stabilityText.c_str());
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
        if (selectedCharacterIndex > 0) {
            ImGui::Text("Personagem: P%d | Era dos Fidalgos 0.2.0", selectedCharacterIndex);
        } else {
            ImGui::Text("Era dos Fidalgos 0.2.0");
        }
        ImGui::PopStyleColor();

    }
    ImGui::End();

    // Restaurar estilo
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(3);
}