#pragma once
#include <string>
#include <unordered_map>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
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

    void DrawRect(const glm::vec2 &position, const glm::vec2 &size,  float rotation,
                  const glm::vec3 &color, const glm::vec2 &cameraPos, RendererMode mode);

    void DrawTexture(const glm::vec2 &position, const glm::vec2 &size,  float rotation,
                     const glm::vec3 &color, Texture *texture,
                     const glm::vec4 &textureRect = glm::vec4(0, 0, 1, 1),
                     const glm::vec2 &cameraPos = glm::vec2(0.0f), bool flip = false,
                     float textureFactor = 1.0f);

    void DrawGeometry(const glm::vec2 &position, const glm::vec2 &size,  float rotation,
                      const glm::vec3 &color, const glm::vec2 &cameraPos, VertexArray *vertexArray, RendererMode mode);

    void Clear();
    void Present();

    class Texture* GetTexture(const std::string& fileName);

	void UpdateOrthographicMatrix(int width, int height);

	glm::vec2 GetWindowSize() const;
	glm::vec2 GetMousePosNDC() const;

	// OpenGL context
	SDL_GLContext mContext;

    void Draw(RendererMode mode, const glm::mat4 &modelMatrix, const glm::vec2 &cameraPos, VertexArray *vertices,
              const glm::vec3 &color,  Texture *texture = nullptr, const glm::vec4 &textureRect = glm::vec4(0, 0, 1, 1), float textureFactor = 1.0f);

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
	glm::mat4 mOrthoProjection;

    // Map of textures loaded
    std::unordered_map<std::string, class Texture*> mTextures;
};