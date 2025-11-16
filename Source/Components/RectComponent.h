//
// Created by Lucas N. Ferreira on 09/10/25.
//

#pragma once

#include "../Renderer/VertexArray.h"
#include "../Math.h"
#include "../Renderer/Renderer.h"

struct RectComponent
{
    RectComponent(class Actor* owner, int width, int height, RendererMode mode, int drawOrder = 100);
    ~RectComponent();

    void Draw(class Renderer* renderer, Vector2 position, float rotation, Vector2 cameraPos);

    int mWidth;
    int mHeight;
    RendererMode mMode;
};
