#pragma once

#include <flecs.h>
#include <glm/glm.hpp>

#include "../Renderer/Renderer.h"
#include "../Components/Window.h"
#include "../Components/Camera.h"
#include "glm/ext/matrix_transform.hpp"

namespace ChessBoardNS
{
    struct ChessBoardScene
    {
    };

    struct ChessBoard
    {
        // The model matrix for the board (identity for now)
        const glm::mat4 mModel = glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1000.0f, 1.0f));

        // Draw the checkerboard using the current camera matrices
        void Draw(Renderer& renderer, const Camera& camera, const Window& window);

    };

    void RegisterSystems(const flecs::world &ecs);
    void Initialize(const flecs::world &ecs);
    void Destroy(const flecs::world &ecs);
}

