// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include <algorithm>
#include <vector>
#include <map>
#include <fstream>
#include "CSV.h"
#include "Game.h"
#include "Components/Drawing/DrawComponent.h"
#include "Components/Physics/RigidBodyComponent.h"
#include "Random.h"
#include "Actors/Actor.h"

Game::Game()
        :mWindow(nullptr)
        ,mRenderer(nullptr)
        ,mTicksCount(0)
        ,mIsRunning(true)
        ,mIsDebugging(true)
        ,mUpdatingActors(false)
        ,mCameraPos(Vector2::Zero)
        ,mMario(nullptr)
        ,mLevelData(nullptr)
{

}

bool Game::Initialize()
{
    Random::Init();
    mIsRunning = true;
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow("TP3: Super Mario Bros", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
    if (!mWindow)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    mRenderer = new Renderer(mWindow);
    mRenderer->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT);

    // Init all game actors
    InitializeActors();

    mTicksCount = SDL_GetTicks();

    return true;
}

void Game::InitializeActors()
{
    auto level = LoadLevel("../Assets/Levels/Level1-1/level1-1.csv", 215, 15);
    if (level == nullptr) {
        mIsRunning = false;
        printf("Failure to load level\n");
    }
    else {
        BuildLevel(level, LEVEL_WIDTH, LEVEL_HEIGHT);
    }
}

int **Game::LoadLevel(const std::string& fileName, int width, int height)
{

    std::ifstream file(fileName);
    if (!file.is_open()) {
        printf("Failed to open file\n");
        return nullptr;
    }

    const auto level = new int*[width];
    for (int i = 0; i < width; i++)
        level[i] = new int[height];

    std::string line;
    int row = 0;
    while (row < height && std::getline(file, line)) {
        std::vector<int> rowTiles = CSVHelper::Split(line);
        if (rowTiles.size() != width)
            return nullptr;

        for (int j = 0; j < width; j++)
            level[j][row] = rowTiles.at(j);
        row++;
    }

    return row < height-1 ? nullptr : level;
}

void Game::BuildLevel(int** levelData, int width, int height)
{
    new Background(this, "../Assets/Sprites/Background.png");

    auto offset = Vector2::One * TILE_SIZE * .5f;
    for (int i=0; i < width; i++) {
        for (int j=0; j < height; j++) {
            Actor* actor= nullptr;
            switch (levelData[i][j]) {

                case 0: {
                    actor = new Block(this, "../Assets/Sprites/Blocks/BlockA.png");
                    break;
                }

                case 16: {
                    mMario = new Mario(this);
                    actor = mMario;
                    break;
                }
                case 1: {
                    bool mushroom = false;
                    if (j>0)
                        mushroom = levelData[i][j-1] == 13;
                    actor = new MysteryBlock(this,
                        "../Assets/Sprites/Blocks/BlockC.png",
                        "../Assets/Sprites/Blocks/BlockE.png",
                        mushroom);
                    break;
                }
                case 4: {
                    auto b = new Block(this, "../Assets/Sprites/Blocks/BlockB.png");
                    b->SetBreakable(true);
                    actor = b;
                    break;
                }
                case 8: {
                    actor = new Block(this, "../Assets/Sprites/Blocks/BlockD.png");
                    break;
                }


                case 10: {
                    actor = new Spawner(this, 15 * TILE_SIZE);
                    break;
                }

                // Canes
                case 12: {
                    actor = new Block(this, "../Assets/Sprites/Blocks/BlockG.png");
                    break;
                }
                case 2: {
                    actor = new Block(this, "../Assets/Sprites/Blocks/BlockF.png");
                    break;
                }
                case 9: {
                    actor = new Block(this, "../Assets/Sprites/Blocks/BlockH.png");
                    break;
                }
                case 6: {
                    actor = new Block(this, "../Assets/Sprites/Blocks/BlockI.png");
                    break;
                }
                default: {
                    break;
                }
            }

            if (actor != nullptr) {
                actor->SetPosition(Vector2(i * Game::TILE_SIZE, j * Game::TILE_SIZE) + offset);
                actor->SetScale(actor->GetScale() * Vector2(Game::TILE_SIZE, Game::TILE_SIZE));
                actor->SetRotation(Math::Pi);
            }
        }
    }
}

void Game::RunLoop()
{

    while (mIsRunning)
    {
        // Calculate delta time in seconds
        float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;
        if (deltaTime > 0.05f)
        {
            deltaTime = 0.05f;
        }

        mTicksCount = SDL_GetTicks();

        ProcessInput();
        UpdateGame(deltaTime);
        GenerateOutput();

        // Sleep to maintain frame rate
        int sleepTime = (1000 / FPS) - (SDL_GetTicks() - mTicksCount);
        if (sleepTime > 0)
        {
            SDL_Delay(sleepTime);
        }
    }
}

void Game::ProcessInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                Quit();
                break;
            default:
                break;
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);
    if (state[SDL_SCANCODE_ESCAPE]) {
        Quit();
        return;
    }
    for (const auto actor : mActors)
    {
        actor->ProcessInput(state);
    }
}

void Game::UpdateGame(float deltaTime)
{
    // Update all actors and pending actors
    UpdateActors(deltaTime);

    // Update camera position
    UpdateCamera();
}

void Game::UpdateActors(float deltaTime)
{
    mUpdatingActors = true;
    for (auto actor : mActors)
    {
        actor->Update(deltaTime);
    }
    mUpdatingActors = false;

    for (auto pending : mPendingActors)
    {
        mActors.emplace_back(pending);
    }
    mPendingActors.clear();

    std::vector<Actor*> deadActors;
    for (auto actor : mActors)
    {
        if (actor->GetState() == ActorState::Destroy)
        {
            deadActors.emplace_back(actor);
        }
    }

    for (auto actor : deadActors)
    {
        delete actor;
    }
}

void Game::UpdateCamera()
{
    auto pos = mMario->GetPosition();
    if ((mCameraPos.x < pos.x - TILE_SIZE * 10) && (pos.x < TILE_SIZE * (Game::LEVEL_WIDTH  - 10.f)))
        mCameraPos.Set(pos.x - TILE_SIZE * 10, mCameraPos.y);
    if (mCameraPos.y > pos.y - TILE_SIZE * (5.f))
        mCameraPos.Set(mCameraPos.x, pos.y - TILE_SIZE * 5);
    else if (mCameraPos.y < 0 && mCameraPos.y < pos.y - TILE_SIZE * 5)
        mCameraPos.Set(mCameraPos.x, pos.y - TILE_SIZE * 5);
}

void Game::AddActor(Actor* actor)
{
    if (mUpdatingActors)
    {
        mPendingActors.emplace_back(actor);
    }
    else
    {
        mActors.emplace_back(actor);
    }
}

void Game::RemoveActor(Actor* actor)
{
    auto iter = std::find(mPendingActors.begin(), mPendingActors.end(), actor);
    if (iter != mPendingActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mPendingActors.end() - 1);
        mPendingActors.pop_back();
    }

    iter = std::find(mActors.begin(), mActors.end(), actor);
    if (iter != mActors.end())
    {
        // Swap to end of vector and pop off (avoid erase copies)
        std::iter_swap(iter, mActors.end() - 1);
        mActors.pop_back();
    }
}

void Game::AddDrawable(class DrawComponent *drawable)
{
    mDrawables.emplace_back(drawable);

    std::sort(mDrawables.begin(), mDrawables.end(),[](DrawComponent* a, DrawComponent* b) {
        return a->GetDrawOrder() < b->GetDrawOrder();
    });
}

void Game::RemoveDrawable(class DrawComponent *drawable)
{
    auto iter = std::find(mDrawables.begin(), mDrawables.end(), drawable);
    mDrawables.erase(iter);
}

void Game::AddCollider(class AABBColliderComponent* collider)
{
    mColliders.emplace_back(collider);
}

void Game::RemoveCollider(AABBColliderComponent* collider)
{
    auto iter = std::find(mColliders.begin(), mColliders.end(), collider);
    mColliders.erase(iter);
}

void Game::GenerateOutput()
{
    // Clear back buffer
    mRenderer->Clear();

    for (auto drawable : mDrawables)
    {
        drawable->Draw(mRenderer);

        if(mIsDebugging)
        {
           // Call draw for actor components
              for (auto comp : drawable->GetOwner()->GetComponents())
              {
                comp->DebugDraw(mRenderer);
              }
        }
    }

    // Swap front buffer and back buffer
    mRenderer->Present();
}

void Game::Shutdown()
{
    while (!mActors.empty()) {
        delete mActors.back();
    }

    // Delete level data
    if (mLevelData) {
        for (int i = 0; i < LEVEL_HEIGHT; ++i) {
            delete[] mLevelData[i];
        }
        delete[] mLevelData;
        mLevelData = nullptr;
    }

    mRenderer->Shutdown();
    delete mRenderer;
    mRenderer = nullptr;

    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}