#include "GameBoard.hpp"

#include <algorithm>

#include "Army.hpp"
#include "Characters.hpp"
#include "Diplomacy.hpp"
#include "DrawProvinces.hpp"
#include "imgui.h"
#include "Game.hpp"
#include "GameTime.hpp"
#include "Components/Province.hpp"
#include "Renderer/Shader.hpp"
#include "Systems/EstatePower.hpp"  // Para acessar EstatePowers
#include "Random.hpp"  // Para Random::GetIntRange

constexpr int BUILDING_COST = 3000;

GameBoardScene::GameBoardScene(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTickSources>();
    const flecs::entity tickTimer = timers.mTickTimer;

    void(ecs.system<Camera, const InputState, const Window>("UpdateCamera")
        .tick_source(tickTimer)
        .kind(flecs::OnLoad)
        .each([](const flecs::iter& it, size_t, Camera &camera, const InputState &input, const Window &window)
        {
            UpdateCamera(camera, input, window, it.delta_time());
        }));

    ecs.system<const Province, const InputState>("SetProvinceShowDetails")
        .with<Hovered>()
        .each([](flecs::entity entity, const Province &, const InputState &input)
        {
            if (input.Clicked) void(entity.add<ShowProvinceDetails>());
        });

    ecs.system<const Province, const ProvinceArmy, const Title>("HoverProvinceName")
        .term_at(2).src("$title")
        .with<Hovered>()
        .with<InRealm>("$title")
        .tick_source(tickTimer)
        .each([](const Province &province, const ProvinceArmy &army, const Title &title)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;
            ImGui::BeginTooltip();

            ImGui::Text("%s - %s", province.name.data(), title.name.data());
            ImGui::Text("Posicao: (%zu, %zu)", province.mPosX, province.mPosY);
            ImGui::Text("%u Tropas", army.mAmount);

            // Informacoes adicionais no tooltip
            ImGui::Separator();
            ImGui::Text("Desenvolvimento: %zu", province.development);
            ImGui::Text("Controle: %.1f%%", static_cast<double>(province.control));
            ImGui::Text("Renda: %.2f ducados anuais", province.IncomeFloat());

            ImGui::EndTooltip();
        });

    ecs.system<Province, ProvinceArmy, const Title, Character>("ShowProvinceDetails")
        .term_at(2).src("$title")
        .term_at(3).src("$character")
        .with<InRealm>("$title")
        .with<RuledBy>("$character").src("$title")
        .with<ShowProvinceDetails>()
        .tick_source(tickTimer)
        .each([ecs](flecs::iter it, size_t i, Province &province, ProvinceArmy &army, const Title &title, Character &character)
        {
            flecs::entity entity = it.entity(i);
            flecs::world world = it.world();
            flecs::entity characterEntity = it.get_var("character");  // ADICIONADO

            // Verificar se esta provincia e a capital do titulo
            flecs::entity titleEntity = it.get_var("title");
            bool isCapital = entity.has<CapitalOf>(titleEntity);

            bool open = true;
            const auto windowTitle = "Provincia Selecionada##" + std::to_string(entity.id());
            const ImVec2 center = ImGui::GetMainViewport()->GetCenter();

            // MODIFICADO: Tamanho da janela ajustado para incluir construções
            ImGui::SetNextWindowSize(ImVec2(450, isCapital ? 600 : 550), ImGuiCond_Appearing);
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});

            // Estilo da janela
            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.12f, 0.95f));
            ImGui::PushStyleColor(ImGuiCol_Border, isCapital ?
                ImVec4(0.9f, 0.7f, 0.1f, 1.0f) :  // Dourado para capital
                ImVec4(0.4f, 0.6f, 0.8f, 1.0f));  // Azul para outras
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, isCapital ? 3.0f : 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

            if (ImGui::Begin(windowTitle.data(), &open,
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse))
            {
                ImGui::Text("Nome: %s", province.name.data());
                ImGui::Text("Parte de: %s", title.name.data());
                ImGui::Text("Governada por: %s", character.mName.data());
                ImGui::Text("%d tropas estacionadas", army.mAmount);
                // Cabecalho especial para capital
                if (isCapital) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                    ImGui::SetWindowFontScale(1.2f);
                    ImGui::Text("[CAPITAL] %s", province.name.data());
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::PopStyleColor();

                    // Indicador de capital
                    ImGui::SameLine();
                    ImGui::Text("(*)");

                    // Tooltip para capital
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("Esta e a capital do reino!");
                        ImGui::Text("Centro politico e administrativo.");
                        ImGui::EndTooltip();
                    }
                } else {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 1.0f, 1.0f));
                    ImGui::Text("%s", province.name.data());
                    ImGui::PopStyleColor();
                }

                ImGui::Separator();
                ImGui::Spacing();

                // Secao: Informacoes Gerais
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
                ImGui::Text("INFORMACOES GERAIS");
                ImGui::PopStyleColor();

                ImGui::Columns(2, "provinceInfo", false);
                ImGui::SetColumnWidth(0, 150);

                ImGui::Text("Dominio:");
                ImGui::NextColumn();
                ImGui::Text("%s", title.name.data());
                ImGui::NextColumn();

                ImGui::Text("Governante:");
                ImGui::NextColumn();
                ImGui::Text("%s", character.mName.data());
                ImGui::NextColumn();

                ImGui::Text("Posicao:");
                ImGui::NextColumn();
                ImGui::Text("(%zu, %zu)", province.mPosX, province.mPosY);
                ImGui::NextColumn();

                //ImGui::Text("Cultura:");
                //ImGui::NextColumn();
                //ImGui::Text("%s", GetCultureName(province.culture).c_str());
                //ImGui::NextColumn();

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Secao: Estatisticas da Provincia
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
                ImGui::Text("ESTATISTICAS");
                ImGui::PopStyleColor();

                ImGui::Columns(2, "provinceStats", false);
                ImGui::SetColumnWidth(0, 150);

                // Desenvolvimento com barra de progresso
                ImGui::Text("Desenvolvimento:");
                ImGui::NextColumn();
                float developmentProgress = static_cast<float>(province.development) / 20.0f;
                ImGui::ProgressBar(developmentProgress, ImVec2(-FLT_MIN, 0),
                    std::to_string(province.development).c_str());
                ImGui::NextColumn();

                // Controle com barra colorida
                ImGui::Text("Controle:");
                ImGui::NextColumn();
                ImVec4 controlColor;
                if (province.control >= 70) controlColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
                else if (province.control >= 40) controlColor = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);
                else controlColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

                float controlProgress = province.control / 100.0f;
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, controlColor);
                ImGui::ProgressBar(controlProgress, ImVec2(-FLT_MIN, 0),
                    (std::to_string(static_cast<int>(province.control)) + "%").c_str());
                ImGui::PopStyleColor();
                ImGui::NextColumn();

                // Renda
                ImGui::Text("Renda Diaria:");
                ImGui::NextColumn();
                ImGui::Text("%.2f ouros", province.IncomeFloat());
                ImGui::NextColumn();

                // Distancia ate a capital (se nao for capital)
                if (!isCapital) {
                    ImGui::Text("Dist. Capital:");
                    ImGui::NextColumn();
                    ImGui::Text("%.1f", static_cast<double>(province.distance_to_capital));
                    ImGui::NextColumn();
                }

                // Opiniao popular
                ImGui::Text("Opiniao Popular:");
                ImGui::NextColumn();
                ImVec4 opinionColor;
                if (province.popular_opinion >= 5) opinionColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
                else if (province.popular_opinion >= -5) opinionColor = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);
                else opinionColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);

                ImGui::TextColored(opinionColor, "%d", static_cast<int>(province.popular_opinion));
                ImGui::NextColumn();

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // ============================================================
                // ADICIONADO: SEÇÃO DE CONSTRUÇÕES
                // ============================================================

                if (characterEntity.has<Player>()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
                    ImGui::Text("CONSTRUCOES");
                    ImGui::PopStyleColor();

                    ImGui::Spacing();

                    // Estradas
                    ImGui::Text("Estradas         ");
                    ImGui::SameLine();
                    if (ImGui::Button("-##1") && province.roads_level > 0) {
                        province.roads_level--;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%u", province.roads_level);
                    ImGui::SameLine();
                    if (ImGui::Button("+##1") && province.roads_level < 5 && character.mMoney >= BUILDING_COST) {
                        character.mMoney -= BUILDING_COST;
                        province.roads_level++;
                    }

                    // Fortificação
                    ImGui::Text("Fortificacao     ");
                    ImGui::SameLine();
                    if (ImGui::Button("-##2") && province.fortification_level > 0) {
                        province.fortification_level--;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%u", province.fortification_level);
                    ImGui::SameLine();
                    if (ImGui::Button("+##2") && province.fortification_level < 5 && character.mMoney >= BUILDING_COST) {
                        character.mMoney -= BUILDING_COST;
                        province.fortification_level++;
                    }

                    // Mercados
                    ImGui::Text("Mercados         ");
                    ImGui::SameLine();
                    if (ImGui::Button("-##3") && province.market_level > 0) {
                        province.market_level--;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%u", province.market_level);
                    ImGui::SameLine();
                    if (ImGui::Button("+##3") && province.market_level < 5 && character.mMoney >= BUILDING_COST) {
                        character.mMoney -= BUILDING_COST;
                        province.market_level++;
                    }

                    // Templos
                    ImGui::Text("Templos          ");
                    ImGui::SameLine();
                    if (ImGui::Button("-##4") && province.temples_level > 0) {
                        province.temples_level--;
                    }
                    ImGui::SameLine();
                    ImGui::Text("%u", province.temples_level);
                    ImGui::SameLine();
                    if (ImGui::Button("+##4") && province.temples_level < 5 && character.mMoney >= BUILDING_COST) {
                        character.mMoney -= BUILDING_COST;
                        province.temples_level++;
                    }

                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Spacing();
                }
                // ============================================================
                // FIM DA SEÇÃO DE CONSTRUÇÕES
                // ============================================================

                // BOTAO ESPECIAL APENAS PARA CAPITAL
                if (isCapital && !characterEntity.has<Player>()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.5f, 1.0f));
                    ImGui::Text("ACOES ESPECIAIS DA CAPITAL");
                    ImGui::PopStyleColor();

                    ImGui::Spacing();

                    // Botao "Provocar"
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                    if (ImGui::Button("[PROVOCAR]", ImVec2(-FLT_MIN, 40))) {
                        // Verificar se o jogador tem ouro suficiente
                        flecs::entity playerEntity = world.entity<Player>();
                        Character* playerChar = playerEntity.try_get_mut<Character>();

                        if (playerChar && playerChar->mMoney >= 100) { // 1.00 ouro = 100 unidades
                            // Gastar 1 ouro
                            playerChar->mMoney -= 100;

                            // Aumentar aleatoriamente 1 poder dos estados
                            EstatePowers* estatePowers = world.try_get_mut<EstatePowers>();
                            if (estatePowers) {
                                int randomEstate = Random::GetIntRange(0, 2);
                                int powerIncrease = Random::GetIntRange(1, 3); // Aumentar 1-3 pontos

                                switch (randomEstate) {
                                    case 0: // Plebe
                                        estatePowers->mCommonersPower = std::clamp(
                                            estatePowers->mCommonersPower + powerIncrease, -128, 127);
                                        SDL_Log("Provocacao: Plebe +%d (Total: %d)",
                                               powerIncrease, estatePowers->mCommonersPower);
                                        break;
                                    case 1: // Nobreza
                                        estatePowers->mNobilityPower = std::clamp(
                                            estatePowers->mNobilityPower + powerIncrease, -128, 127);
                                        SDL_Log("Provocacao: Nobreza +%d (Total: %d)",
                                               powerIncrease, estatePowers->mNobilityPower);
                                        break;
                                    case 2: // Clero
                                        estatePowers->mClergyPower = std::clamp(
                                            estatePowers->mClergyPower + powerIncrease, -128, 127);
                                        SDL_Log("Provocacao: Clero +%d (Total: %d)",
                                               powerIncrease, estatePowers->mClergyPower);
                                        break;
                                }

                                // Mostrar mensagem de feedback
                                ImGui::OpenPopup("Provocacao Realizada");
                            }
                        } else {
                            // Mostrar mensagem de erro se nao tiver ouro
                            ImGui::OpenPopup("Ouro Insuficiente");
                        }
                    }

                    ImGui::PopStyleColor(4);

                    // Tooltip explicativo
                    if (ImGui::IsItemHovered()) {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[PROVOCAR]");
                        ImGui::Separator();
                        ImGui::Text("Gasta 1 ouro para aumentar aleatoriamente");
                        ImGui::Text("o poder de um dos estados.");

                        // Linha em branco
                        ImGui::Spacing();

                        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Efeito:");
                        ImGui::Text("  +1 a +3 pontos em Plebe, Nobreza ou Clero");

                        // Linha em branco
                        ImGui::Spacing();

                        ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.3f, 1.0f), "Custo: 1.00 ouro");
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }

                    ImGui::Spacing();

                    // Popup de sucesso
                    if (ImGui::BeginPopupModal("Provocacao Realizada", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Voce gastou 1 ouro para provocar!");
                        ImGui::Text("O poder de um estado aumentou aleatoriamente.");
                        ImGui::Separator();

                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    // Popup de erro
                    if (ImGui::BeginPopupModal("Ouro Insuficiente", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Ouro insuficiente!");
                        ImGui::Text("Voce precisa de pelo menos 1.00 ouro para provocar.");
                        ImGui::Separator();

                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }

                    ImGui::Separator();
                    ImGui::Spacing();
                }

                // Informacoes do Governante
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f));
                ImGui::Text("INFORMACOES DO GOVERNANTE");
                ImGui::PopStyleColor();

                ImGui::Text("Nome: %s", character.mName.data());
                ImGui::Text("Idade: %zu anos", character.mAgeDays / 360);
                ImGui::Text("Tesouro: %.2f ouros", character.mMoney * 0.01f);

                // Botao de fechar
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::SetCursorPosX((ImGui::GetWindowWidth() - 100) * 0.5f);
                if (ImGui::Button("Fechar", ImVec2(100, 30))) {
                    open = false;
                }

                if (characterEntity.has<Player>())
                {
                    if (ImGui::Button("Mover Exércitos"))
                    {
                        ecs.entity<MovingArmies>().set<MovingArmies>({
                            .mProvince = entity,
                            .mAmount = 0,
                        });
                    }
                    if (ImGui::Button("Comprar Tropas"))
                    {
                        character.mMoney -= 100;
                        army.mAmount += 1;
                    }
                }
            }

            ImGui::End();

            // Restaurar estilo
            ImGui::PopStyleVar(3);
            ImGui::PopStyleColor(2);

            if (!open) void(entity.remove<ShowProvinceDetails>());
        });

    DoRenderTileMapSystem(ecs);
}

void GameBoard::Draw(Renderer& renderer, const Camera &camera, const Window &window)
{
    const auto &chessShader = renderer.mChessShader;
    const auto &verts = renderer.mSpriteVerts;
    if (!chessShader || !verts) return;

    // 1. Set the shader
    chessShader->SetActive();
    // 2. Set the matrices
    chessShader->SetMatrixUniform("uView", camera.mView);
    chessShader->SetMatrixUniform("uProj", camera.CalculateProjection(window));
    chessShader->SetMatrixUniform("uModel", mModel);
    chessShader->SetVectorUniform("uMousePos", window.GetMousePosNDC());

    // 3. Draw the plane
    verts->SetActive();

    glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
}