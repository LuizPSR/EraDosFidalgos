#include <GL/glew.h>
#include "Renderer.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture.h"

Renderer::Renderer(SDL_Window *window)
    : mContext(nullptr)
      , mBaseShader(nullptr)
      , mSpriteVerts(nullptr)
      , mWindow(window)
      , mOrthoProjection(Matrix4::Identity)
{
}

Renderer::~Renderer()
{
    delete mSpriteVerts;
    mSpriteVerts = nullptr;
}

bool Renderer::Initialize()
{


    // Request OpenGL 4.1 Core Profile (4.1 is the highest supported by Apple on recent macOS)
    // NOTE: You can also request 3.2 or 3.3.
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

    // 2. Create the window and OpenGL context (The window must have the SDL_WINDOW_OPENGL flag)
    // (Assuming mWindow is created elsewhere with SDL_WINDOW_OPENGL)
    // mWindow = SDL_CreateWindow(... SDL_WINDOW_OPENGL ...);

    // Create an OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);
    if (!mContext) {
        // Handle error: context creation failed
        return false;
    }
    SDL_GL_MakeCurrent(mWindow, mContext);

    // 3. Initialize GLEW (or GLAD) *after* the context is created
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        // Handle error: GLEW initialization failed
        return false;
    }

    // Check the actual version created
    int major, minor;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);

    // Create an OpenGL context
    mContext = SDL_GL_CreateContext(mWindow);
    SDL_GL_MakeCurrent(mWindow, mContext);

    // --- ADD GLSL VERSION CHECK HERE ---
    const GLubyte* glVersion = glGetString(GL_VERSION);
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

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

    // Create orthographic projection matrix
    const auto &sz = GetWindowSize();
    UpdateOrthographicMatrix(sz.x, sz.y);

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

void Renderer::DrawRect(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                        const Vector2 &cameraPos, RendererMode mode)
{
    Matrix4 model = Matrix4::CreateScale(Vector3(size.x, size.y, 1.0f)) *
        Matrix4::CreateRotationZ(rotation) *
        Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));

    Draw(mode, model, cameraPos, mSpriteVerts, color);
}

void Renderer::DrawTexture(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                           Texture *texture, const Vector4 &textureRect, const Vector2 &cameraPos, bool flip,
                           float textureFactor)
{
    float flipFactor = flip ? -1.0f : 1.0f;

    Matrix4 model = Matrix4::CreateScale(Vector3(size.x * flipFactor, size.y, 1.0f)) *
        Matrix4::CreateRotationZ(rotation) *
        Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));

    Draw(RendererMode::TRIANGLES, model, cameraPos, mSpriteVerts, color, texture, textureRect, textureFactor);
}

void Renderer::DrawGeometry(const Vector2 &position, const Vector2 &size, float rotation, const Vector3 &color,
                            const Vector2 &cameraPos, VertexArray *vertexArray, RendererMode mode)
{
    Matrix4 model = Matrix4::CreateScale(Vector3(size.x, size.y, 1.0f)) *
        Matrix4::CreateRotationZ(rotation) *
        Matrix4::CreateTranslation(Vector3(position.x, position.y, 0.0f));

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
    mOrthoProjection = Matrix4::CreateOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    mBaseShader->SetMatrixUniform("uOrthoProj", mOrthoProjection);
    glViewport(0, 0, width, height);
}

Vector2 Renderer::GetWindowSize() const
{
    int width, height;
    SDL_GetWindowSize(mWindow, &width, &height);
    return Vector2{width, height};
}

void Renderer::Draw(RendererMode mode, const Matrix4 &modelMatrix, const Vector2 &cameraPos, VertexArray *vertices,
                    const Vector3 &color, Texture *texture, const Vector4 &textureRect, float textureFactor)
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
