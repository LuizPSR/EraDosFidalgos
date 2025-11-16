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
    ecs.component<ChessBoardScene>().add(flecs::Singleton);
    ecs.system<Camera, const InputState>("UpdateCamera")
        .kind(flecs::OnLoad)
        .run(UpdateCamera)
        .disable();

    ecs.system<ChessBoard, const Camera, Renderer>("Render3DScene")
        .kind(flecs::PreStore)
        .each(RenderScene)
        .disable();

    ecs.system("SceneUI")
        .kind(flecs::OnUpdate)
        .run([](const flecs::iter &it)
        {
            ImGui::Begin("Scene Loaded");
            ImGui::End();
        })
        .disable();
}

void ChessBoardNS::Initialize(const flecs::world& ecs)
{
    ecs.add<ChessBoardScene>();
    auto scene = ecs.entity<ChessBoardScene>();

    // Add camera to the scene entity
    scene.emplace<Camera>(Camera{
        .mPosition{50.0f, 50.0f, 0.0f},
        .mTarget{0.0f, 0.0f, 0.0f},
        .mZoomLevel = 50.0f,
    });

    // Add board component to the scene
    scene.emplace<ChessBoard>();

    ecs.entity("UpdateCamera").enable();
    ecs.entity("Render3DScene").enable();
    ecs.entity("SceneUI").enable();

    ecs.each([](Camera &camera)
    {
        camera.RecalculateView();
    });
}

void ChessBoardNS::Destroy(const flecs::world& ecs)
{
    ecs.entity<ChessBoardScene>().destruct();
    ecs.entity("UpdateCamera").disable();
    ecs.entity("Render3DScene").disable();
    ecs.entity("SceneUI").disable();
}