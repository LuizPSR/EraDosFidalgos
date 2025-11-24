#pragma once

#include <flecs.h>
#include <glm/glm.hpp>

#include "../Renderer/Renderer.h"
#include "../Components/Window.h"
#include "../Components/Camera.h"
#include "glm/ext/matrix_transform.hpp"

struct ChessBoardScene
{
    explicit ChessBoardScene(const flecs::world &ecs);
    static void Start(const flecs::entity &e);
    static void Stop(const flecs::entity &e);
};

struct ChessBoard
{
    // The model matrix for the board
    const glm::mat4 mModel = glm::scale(glm::mat4(1.0f), glm::vec3(1000.0f, 1000.0f, 1.0f));

    // Draw the checkerboard using the current camera matrices
    void Draw(Renderer& renderer, const Camera& camera, const Window& window);
};
