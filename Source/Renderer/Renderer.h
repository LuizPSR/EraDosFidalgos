#pragma once
#include <string>
#include <unordered_map>
#include <SDL3/SDL.h>
#include "../Math.h"
#include "VertexArray.h"
#include "Texture.h"

enum class RendererMode
{
    TRIANGLES,
    LINES
};

struct Renderer
{
	explicit Renderer(SDL_Window* window);
	~Renderer();

	bool Initialize();
	void Shutdown();

    void DrawRect(const Vector2 &position, const Vector2 &size,  float rotation,
                  const Vector3 &color, const Vector2 &cameraPos, RendererMode mode);

    void DrawTexture(const Vector2 &position, const Vector2 &size,  float rotation,
                     const Vector3 &color, Texture *texture,
                     const Vector4 &textureRect = Vector4::UnitRect,
                     const Vector2 &cameraPos = Vector2::Zero, bool flip = false,
                     float textureFactor = 1.0f);

    void DrawGeometry(const Vector2 &position, const Vector2 &size,  float rotation,
                      const Vector3 &color, const Vector2 &cameraPos, VertexArray *vertexArray, RendererMode mode);

    void Clear();
    void Present();

    // Getters
	// TODO: get rid of this
    class Texture* GetTexture(const std::string& fileName);
	class Shader* GetBaseShader() const { return mBaseShader; }

	void UpdateOrthographicMatrix(int width, int height);

	Vector2 GetWindowSize() const;

	// OpenGL context
	SDL_GLContext mContext;

    void Draw(RendererMode mode, const Matrix4 &modelMatrix, const Vector2 &cameraPos, VertexArray *vertices,
              const Vector3 &color,  Texture *texture = nullptr, const Vector4 &textureRect = Vector4::UnitRect, float textureFactor = 1.0f);

	bool LoadShaders();
    void CreateSpriteVerts();

	// Basic shader
	class Shader* mBaseShader;

	// Chess Shader
	class Shader* mChessShader;

    // Sprite vertex array
    class VertexArray *mSpriteVerts;

	// Window
	SDL_Window* mWindow;

	// Ortho projection for 2D shaders
	Matrix4 mOrthoProjection;

    // Map of textures loaded
    std::unordered_map<std::string, class Texture*> mTextures;
};