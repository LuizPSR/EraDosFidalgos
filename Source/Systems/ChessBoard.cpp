#include "ChessBoard.h"

#include "imgui.h"
#include "SDL3/SDL.h"
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

void ChessBoard::Draw(Renderer& renderer, const Camera& camera)
{
    const auto &chessShader = renderer.mChessShader;
    const auto &verts = renderer.mSpriteVerts;
    if (!chessShader || !verts) return;

    // 1. Set the shader
    chessShader->SetActive();
    // 2. Set the matrices
    chessShader->SetMatrixUniform("uView", camera.mView);
    chessShader->SetMatrixUniform("uProj", camera.CalculateProjection(renderer));
    chessShader->SetMatrixUniform("uModel", mModel);
    // 3. Draw the plane
    verts->SetActive();

    glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
}

void RenderScene(ChessBoard &board, const Camera &camera, Renderer &renderer)
{
    board.Draw(renderer, camera);
}

void ChessBoardNS::RegisterSystems(const flecs::world& ecs)
{
    void(ecs.component<ChessBoardScene>().add(flecs::Singleton));
    void(ecs.system<Camera, const InputState>("UpdateCamera")
        .kind(flecs::OnLoad)
        .run(UpdateCamera)
        .disable());

    void(ecs.system<ChessBoard, const Camera, Renderer>("Render3DScene")
        .kind(flecs::PreStore)
        .each(RenderScene)
        .disable());

    void(ecs.system("SceneUI")
        .kind(flecs::OnUpdate)
        .run([](const flecs::iter &it)
        {
            ImGui::Begin("Scene Loaded");
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