#include "ChessBoard.hpp"

#include "imgui.h"
#include "Game.hpp"
#include "Renderer/Shader.hpp"

ChessBoardScene::ChessBoardScene(const flecs::world& ecs)
{
    const auto &timers = ecs.get<GameTimers>();
    const flecs::entity tickTimer = timers.mTickTimer;

    void(ecs.system<Camera, const InputState, const Window>("UpdateCamera")
        .tick_source(tickTimer)
        .kind(flecs::OnLoad)
        .each([](const flecs::iter& it, size_t, Camera &camera, const InputState &input, const Window &window)
        {
            UpdateCamera(camera, input, window, it.delta_time());
        }));

    void(ecs.system<ChessBoard, const Camera, Renderer, const Window>("Render3DScene")
        .kind(flecs::PreStore)
        .each([](ChessBoard &board, const Camera &camera, Renderer &renderer, const Window &window)
        {
            board.Draw(renderer, camera, window);
        }));

    void(ecs.system<Camera, const Window>("SceneUI")
        .tick_source(tickTimer)
        .kind(flecs::OnUpdate)
        .each([](Camera &camera, const Window &window)
        {
            ImGui::Begin("Scene Loaded");

            ImGui::Text("Camera Position: (%.1f, %.1f, %.1f)", camera.mPosition.x, camera.mPosition.y, camera.mPosition.z);
            ImGui::Text("Target Position: (%.1f, %.1f, %.1f)", camera.mTarget.x, camera.mTarget.y, camera.mTarget.z);
            ImGui::SliderFloat("Zoom", &camera.mZoomLevel, camera.mMinZoom, camera.mMaxZoom);

            glm::vec2 mousePos = window.GetMousePosNDC();
            ImGui::Text("Mouse Position (NDC): (%.1f, %.1f)", mousePos.x, mousePos.y);

            glm::vec3 mouseWorldPos = camera.NDCToWorld(mousePos, window);
            ImGui::Text("Mouse Position (World): (%.1f, %.1f, %.1f)", mouseWorldPos.x, mouseWorldPos.y, mouseWorldPos.z);
            ImGui::End();
        }));

    void(ecs.prefab("Active").add<ChessBoard>().add<Camera>());
}

void ChessBoardScene::Start(const flecs::entity& e)
{
    void(e.is_a(e.lookup("Active")));
}

void ChessBoardScene::Stop(const flecs::entity& e)
{
    void(e.remove(flecs::IsA, e.lookup("Active")));
}

void ChessBoard::Draw(Renderer& renderer, const Camera &camera, const Window &window)
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
