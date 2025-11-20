//
// Created by Lucas N. Ferreira on 09/10/25.
//

#pragma once

#include <glm/glm.hpp>
#include "../Renderer/VertexArray.h"
#include "../Renderer/Renderer.h"

struct RectComponent
{
    RectComponent(class Actor* owner, int width, int height, RendererMode mode, int drawOrder = 100);
    ~RectComponent();

    void Draw(Renderer* renderer, glm::vec2 position, float rotation, glm::vec2 cameraPos);

    int mWidth;
    int mHeight;
    RendererMode mMode;
};
