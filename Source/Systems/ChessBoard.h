#pragma once

#include <flecs.h>

#include "../Math.h"
#include "../Components/Camera.h"
#include "../Renderer/Renderer.h"

namespace ChessBoardNS
{
    struct ChessBoardScene
    {
    };

    struct ChessBoard
    {
        // The model matrix for the board (identity for now)
        const Matrix4 mModel = Matrix4::CreateScale(1000.0f, 1000.0f, 1.0f);

        // Draw the checkerboard using the current camera matrices
        void Draw(Renderer& renderer, const Camera& camera);

    };

    void RegisterSystems(const flecs::world &ecs);
    void Initialize(const flecs::world &ecs);
    void Destroy(const flecs::world &ecs);
}

