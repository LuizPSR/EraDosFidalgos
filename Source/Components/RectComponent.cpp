//
// Created by Lucas N. Ferreira on 09/10/25.
//

#include "RectComponent.h"
#include "../Renderer/Renderer.h"
#include "../Game.h"

RectComponent::RectComponent(class Actor* owner, int width, int height, RendererMode mode, int drawOrder)
    : mMode(mode)
    , mWidth(width)
    , mHeight(height)
{

}

RectComponent::~RectComponent()
{

}

void RectComponent::Draw(class Renderer* renderer, Vector2 position, float rotation, Vector2 cameraPos)
{
    // TODO: restore behavior
    bool mIsVisible = true;
    Vector3 mColor{255, 0, 0};

    if(mIsVisible)
    {
        renderer->DrawRect(position, Vector2(mWidth, mHeight), rotation, mColor, cameraPos, mMode);
    }
}