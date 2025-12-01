#include "GameBoard.hpp"

#include "Characters.hpp"
#include "DrawProvinces.hpp"
#include "imgui.h"
#include "Game.hpp"
#include "GameTime.hpp"
#include "Components/Province.hpp"
#include "Renderer/Shader.hpp"

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

    ecs.system<const Province, const Title>("HoverProvinceName")
        .term_at(1).src("$title")
        .with<Hovered>()
        .with<InRealm>("$title")
        .tick_source(tickTimer)
        .each([](const Province &province, const Title &title)
        {
            if (ImGui::GetIO().WantCaptureMouse) return;
            ImGui::BeginTooltip();
            ImGui::Text("%s - %s (%zu, %zu)", province.name.data(), title.name.data(), province.mPosX, province.mPosY);
            ImGui::EndTooltip();
        });

    ecs.system<const Province, const Title, const Character>("ShowProvinceDetails")
        .term_at(1).src("$title")
        .term_at(2).src("$character")
        .with<InRealm>("$title")
        .with<RuledBy>("$character").src("$title")
        .with<ShowProvinceDetails>()
        .tick_source(tickTimer)
        .each([](flecs::entity entity, const Province &province, const Title &title, const Character &character)
        {
            bool open = true;
            const auto windowTitle = "Província Selecionada##" + std::to_string(entity.id());
            const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2{0.5f, 0.5f});
            if (ImGui::Begin(windowTitle.data(), &open))
            {
                ImGui::Text("Nome: %s", province.name.data());
                ImGui::Text("Parte de: %s", title.name.data());
                ImGui::Text("Governada por: %s", character.mName.data());
                ImGui::Text("\t%zu anos", character.mAgeDays / 360);
                ImGui::Text("\t%0.2f gold", character.mMoney * 0.01f);
            }
            ImGui::End();
            if (!open) void(entity.remove<ShowProvinceDetails>());
        });

    DoRenderTileMapSystem(ecs);

    // void(ecs.system<ChessBoard, const Camera, Renderer, const Window>("Render3DScene")
    //     .kind(flecs::PreStore)
    //     .each([](ChessBoard &board, const Camera &camera, Renderer &renderer, const Window &window)
    //     {
    //         board.Draw(renderer, camera, window);
    //     }));

    // void(ecs.system<Camera, const Window>("SceneUI")
    //     .tick_source(tickTimer)
    //     .kind(flecs::OnUpdate)
    //     .each([](Camera &camera, const Window &window)
    //     {
    //         ImGui::Begin("Scene Loaded");
    //
    //         ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.mPosition.x, camera.mPosition.y, camera.mPosition.z);
    //         ImGui::Text("Target Position: (%.1f, %.1f, %.1f)", camera.mTarget.x, camera.mTarget.y, camera.mTarget.z);
    //         ImGui::SliderFloat("Zoom", &camera.mZoomLevel, camera.mMinZoom, camera.mMaxZoom);
    //
    //         glm::vec2 mousePos = window.GetMousePosNDC();
    //         ImGui::Text("Mouse Position (NDC): (%.1f, %.1f)", mousePos.x, mousePos.y);
    //
    //         glm::vec3 mouseWorldPos = camera.NDCToWorld(mousePos, window);
    //         ImGui::Text("Mouse Position (World): (%.1f, %.1f, %.1f)", mouseWorldPos.x, mouseWorldPos.y, mouseWorldPos.z);
    //         ImGui::End();
    //     }));
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
