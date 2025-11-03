//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once
#include "../Component.h"
#include "../../Math.h"
#include "RigidBodyComponent.h"
#include <vector>
#include <set>

enum class ColliderLayer
{
    Player,
    Enemy,
    Blocks,
    PowerUp,
};

class AABBColliderComponent : public Component
{
public:

    AABBColliderComponent(class Actor* owner, int dx, int dy, int w, int h, int updateOrder = 10);
    ~AABBColliderComponent() override;

    bool DetectClick(Vector2 *position);

    Vector2 GetMin() const;
    Vector2 GetMax() const;

    // Drawing for debug purposes
    void DebugDraw(class Renderer* renderer) override;

    void SetBounds(int dx, int dy, int w, int h);

private:

    Vector2 mOffset;
    int mWidth;
    int mHeight;
    bool mIsStatic;
};