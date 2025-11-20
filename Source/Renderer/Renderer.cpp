#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

Renderer::Renderer(SDL_Window *window)
    : mContext(nullptr)
      , mBaseShader(nullptr)
      , mSpriteVerts(nullptr)
      , mWindow(window)
{
}

Renderer::~Renderer()
{
    delete mSpriteVerts;
    mSpriteVerts = nullptr;
}

bool Renderer::Initialize()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // CRITICAL FIX FOR MACOS: Force Forward Compatibility for modern contexts
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    // Enable double buffering and set the buffer size (standard)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create an OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext) {
        // Handle error: context creation failed
        return false;
    }
    SDL_GL_MakeCurrent(mWindow, mContext);

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        // Handle error: GLEW initialization failed
        return false;
    }

    // Make sure we can create/compile shaders
    if (!LoadShaders()) {
        SDL_Log("Failed to load shaders.");
        return false;
    }

    // Create quad for drawing sprites
    CreateSpriteVerts();

    // Set the clear color to light grey
    glClearColor(0.419f, 0.549f, 1.0f, 1.0f);

    // Enable alpha blending on textures
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create the texture uniform
    mBaseShader->SetIntegerUniform("uTexture", 0);

    // Activate shader
    mBaseShader->SetActive();

    return true;
}

void Renderer::Shutdown()
{
    // Destroy textures
    for (auto i : mTextures)
    {
        i.second->Unload();
        delete i.second;
    }
    mTextures.clear();

    mBaseShader->Unload();
    delete mBaseShader;

    mChessShader->Unload();
    delete mChessShader;

    SDL_GL_DestroyContext(mContext);
    SDL_DestroyWindow(mWindow);
}

void Renderer::DrawRect(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec3 &color,
                        const glm::vec2 &cameraPos, RendererMode mode)
{
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    Draw(mode, model, cameraPos, mSpriteVerts, color);
}

void Renderer::DrawTexture(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec3 &color,
                           Texture *texture, const glm::vec4 &textureRect, const glm::vec2 &cameraPos, bool flip,
                           float textureFactor)
{
    float flipFactor = flip ? -1.0f : 1.0f;

    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size.x * flipFactor, size.y, 1.0f));
    Draw(RendererMode::TRIANGLES, model, cameraPos, mSpriteVerts, color, texture, textureRect, textureFactor);
}

void Renderer::DrawGeometry(const glm::vec2 &position, const glm::vec2 &size, float rotation, const glm::vec3 &color,
                            const glm::vec2 &cameraPos, VertexArray *vertexArray, RendererMode mode)
{
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    Draw(mode, model, cameraPos, vertexArray, color);
}

void Renderer::Clear()
{
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Present()
{
    // Swap the buffers
    SDL_GL_SwapWindow(mWindow);
}

Texture* Renderer::GetTexture(const std::string& fileName)
{
    Texture* tex = nullptr;
    auto iter = mTextures.find(fileName);
    if (iter != mTextures.end())
    {
        tex = iter->second;
    }
    else
    {
        tex = new Texture();
        if (tex->Load(fileName))
        {
            mTextures.emplace(fileName, tex);
            return tex;
        }
        else
        {
            delete tex;
            return nullptr;
        }
    }
    return tex;
}

void Renderer::UpdateOrthographicMatrix(int width, int height)
{
    const float x = static_cast<float>(width), y = static_cast<float>(height);
    mOrthoProjection = glm::ortho(x * -.5f, x * .5f, y * -.5f, y * .5f, -1.0f, 1.0f);
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    glViewport(0, 0, width, height);
}

glm::vec2 Renderer::GetWindowSize() const
{
    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    return glm::vec2{width, height};
}

glm::vec2 Renderer::GetMousePosNDC() const
{
    const glm::vec2 sz = GetWindowSize();
    glm::vec2 mousePos;
    SDL_GetMouseState(&mousePos.x, &mousePos.y);
    mousePos.x = 2.0f * mousePos.x / sz.x - 1.0f;
    mousePos.y = 1.0f - 2.0f * mousePos.y / sz.y;
    return mousePos;
}

void Renderer::Draw(RendererMode mode, const glm::mat4 &modelMatrix, const glm::vec2 &cameraPos, VertexArray *vertices,
                    const glm::vec3 &color, Texture *texture, const glm::vec4 &textureRect, float textureFactor)
{
    mBaseShader->SetMatrixUniform("uWorldTransform", modelMatrix);
    mBaseShader->SetVectorUniform("uColor", color);
    mBaseShader->SetVectorUniform("uTexRect", textureRect);
    mBaseShader->SetVectorUniform("uCameraPos", cameraPos);

    if(vertices)
    {
        vertices->SetActive();
    }

    if(texture)
    {
        texture->SetActive();
        mBaseShader->SetFloatUniform("uTextureFactor", textureFactor);
    }
    else {
        mBaseShader->SetFloatUniform("uTextureFactor", 0.0f);
    }

    if (mode == RendererMode::LINES)
    {
        glDrawElements(GL_LINE_LOOP, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
    }
    else if(mode == RendererMode::TRIANGLES)
    {
        glDrawElements(GL_TRIANGLES, vertices->GetNumIndices(), GL_UNSIGNED_INT,nullptr);
    }
}


bool Renderer::LoadShaders()
{
    // Create sprite shader
    mBaseShader = new Shader();
    if (!mBaseShader->Load("../Shaders/Base")) {
        return false;
    }

    mChessShader = new Shader();
    if (!mChessShader->Load("../Shaders/Chess")) {
        return false;
    }

    mBaseShader->SetActive();

    return true;
}

void Renderer::CreateSpriteVerts()
{
    constexpr float vertices[] = {
        //   POSITION | TEXTURE
        .5f,  .5f,        1.0f, 0.0f,
        .5f, -.5f,        1.0f, 1.0f,
        -.5f, -.5f,        0.0f, 1.0f,
        -.5f,  .5f,        0.0f, 0.0f
    };
    const unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };
    mSpriteVerts = new VertexArray(vertices, 4, indices, 6);
}
