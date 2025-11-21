#include "ChessBoard.h"

#include "imgui.h"
#include "../Renderer/Shader.h"
#include "../Game.h"

using namespace ChessBoardNS;

VertexArray *CreatePlane2DVertexArray()
{
    constexpr float vertices[] = {
        //   POSITION | TEXTURE
        .5f,  .5f,        1.0f, 0.0f,
        .5f, -.5f,        1.0f, 1.0f,
        -.5f, -.5f,        0.0f, 1.0f,
        -.5f,  .5f,        0.0f, 0.0f
    };
    const unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    return new VertexArray(vertices, 4, indices, 6);
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

void RenderScene(ChessBoard &board, const Camera &camera, Renderer &renderer, const Window &window)
{
    board.Draw(renderer, camera, window);
}

void ChessBoardNS::RegisterSystems(const flecs::world& ecs)
{
    void(ecs.component<ChessBoardScene>().add(flecs::Singleton));

    void(ecs.system<Camera, const InputState, const Window>("UpdateCamera")
        .kind(flecs::OnLoad)
        .run([](flecs::iter& it)
        {
            const float deltaTime = it.delta_time();
            while (it.next()) {
                for (const auto i : it) {
                    auto &camera = it.field_at<Camera>(0, i);
                    auto &input = it.field_at<const InputState>(1, i);
                    auto &window = it.field_at<const Window>(2, i);
                    UpdateCamera(camera, input, window, deltaTime);
                }
            }
        })
        .disable());

    void(ecs.system<ChessBoard, const Camera, Renderer, const Window>("Render3DScene")
        .kind(flecs::PreStore)
        .each(RenderScene)
        .disable());

    void(ecs.system<Camera, const Window>("SceneUI")
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
        })
        .disable());
}

void ChessBoardNS::Initialize(const flecs::world& ecs)
{
    ecs.add<ChessBoardScene>();
    auto scene = ecs.entity<ChessBoardScene>();

    // Add camera to the scene entity
    void(scene.add<Camera>());

    // Add board component to the scene
    void(scene.add<ChessBoard>());

    void(ecs.entity("UpdateCamera").enable());
    void(ecs.entity("Render3DScene").enable());
    void(ecs.entity("SceneUI").enable());
}

void ChessBoardNS::Destroy(const flecs::world& ecs)
{
    ecs.entity<ChessBoardScene>().destruct();
    void(ecs.entity("UpdateCamera").disable());
    void(ecs.entity("Render3DScene").disable());
    void(ecs.entity("SceneUI").disable());
}