//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct AnimatorComponent {
    // (Lower draw order corresponds with further back)
    AnimatorComponent(class Renderer* renderer, const std::string &texturePath, const std::string &dataPath,
            int width, int height, int drawOrder = 100);
    ~AnimatorComponent();

    void Draw(class Renderer* renderer, glm::vec2 position, float rotation, glm::vec2 cameraPos);
    void Update(float deltaTime);

    // Use to change the FPS of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    // Set the current active animation
    void SetAnimation(const std::string& name);

    // Use to pause/unpause the animation
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    // Add an animation of the corresponding name to the animation map
    void AddAnimation(const std::string& name, const std::vector<int>& images);

    void SetTextFactor(const float factor) { mTextureFactor = factor;}

    bool LoadSpriteSheetData(const std::string& dataPath);

    // Sprite sheet texture
    class Texture* mSpriteTexture;

    // Vector of sprites
    std::vector<glm::vec4> mSpriteSheetData;

    // Map of animation name to vector of textures corresponding to the animation
    std::unordered_map<std::string, std::vector<int>> mAnimations;

    // Name of current animation
    std::string mAnimName;

    // Tracks current elapsed time in animation
    float mAnimTimer = 0.0f;

    // The frames per second the animation should run at
    float mAnimFPS = 10.0f;

    // Whether or not the animation is paused (defaults to false)
    bool mIsPaused = false;

    // Size
    int mWidth;
    int mHeight;

    float mTextureFactor;
};
