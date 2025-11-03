//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "AnimatorComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Json.h"
#include "../../Renderer/Texture.h"
#include <fstream>

AnimatorComponent::AnimatorComponent(class Actor* owner, const std::string &texPath, const std::string &dataPath,
                                     int width, int height, int drawOrder)
        :DrawComponent(owner,  drawOrder)
        ,mAnimTimer(0.0f)
        ,mIsPaused(false)
        ,mWidth(width)
        ,mHeight(height)
        ,mTextureFactor(1.0f)
{
    const auto renderer = owner->GetGame()->GetRenderer();
    mSpriteTexture = renderer->GetTexture(texPath);
    if (!dataPath.empty())
        LoadSpriteSheetData(dataPath);
}

AnimatorComponent::~AnimatorComponent()
{
    mAnimations.clear();
    mSpriteSheetData.clear();
}

bool AnimatorComponent::LoadSpriteSheetData(const std::string& dataPath)
{
    // Load sprite sheet data and return false if it fails
    std::ifstream spriteSheetFile(dataPath);

    if (!spriteSheetFile.is_open()) {
        SDL_Log("Failed to open sprite sheet data file: %s", dataPath.c_str());
        return false;
    }

    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile);

    if (spriteSheetData.is_null()) {
        SDL_Log("Failed to parse sprite sheet data file: %s", dataPath.c_str());
        return false;
    }

    auto textureWidth = static_cast<float>(spriteSheetData["meta"]["size"]["w"].get<int>());
    auto textureHeight = static_cast<float>(spriteSheetData["meta"]["size"]["h"].get<int>());

    for(const auto& frame : spriteSheetData["frames"]) {

        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();

        mSpriteSheetData.emplace_back(static_cast<float>(x)/textureWidth, static_cast<float>(y)/textureHeight,
                                      static_cast<float>(w)/textureWidth, static_cast<float>(h)/textureHeight);
    }

    return true;
}

void AnimatorComponent::Draw(Renderer* renderer)
{
    if (mIsVisible) {
        const auto game = mOwner->GetGame();

        auto rect = Vector4::UnitRect;
        if (!mAnimations.empty()) {
            try {
                rect = mSpriteSheetData.at(
                    mAnimations.at(mAnimName).at(static_cast<int>(mAnimTimer))
                );
            } catch (const std::out_of_range& e) {}
        }

        renderer->DrawTexture(
            mOwner->GetPosition(), mOwner->GetScale(), mOwner->GetRotation(),
            mColor,
            mSpriteTexture, rect, game->GetCameraPos(),
            true, mTextureFactor
            );
    }
}

void AnimatorComponent::Update(const float deltaTime)
{
    if (mIsPaused || mAnimations.empty())
        return;

    mAnimTimer += deltaTime * mAnimFPS;
    while (mAnimTimer > mAnimations[mAnimName].size() ) {
        mAnimTimer -= mAnimations[mAnimName].size();
    }
}

void AnimatorComponent::SetAnimation(const std::string& name)
{
    if (mAnimName == name)
        return;



    mAnimName = name;
    Update(0.0f);
}

void AnimatorComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    mAnimations.try_emplace(name, spriteNums);
}