#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "VertexArray.h"
#include "Texture.h"
#include "../Components/Window.h"

enum class RendererMode
{
    TRIANGLES,
    LINES
};

struct Renderer
{
	explicit Renderer();
	~Renderer();

	bool Initialize(const Window &window);
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

	// OpenGL context
	SDL_GLContext mContext = nullptr;

    void Draw(RendererMode mode, const glm::mat4 &modelMatrix, const glm::vec2 &cameraPos, VertexArray *vertices,
              const glm::vec3 &color,  Texture *texture = nullptr, const glm::vec4 &textureRect = glm::vec4(0, 0, 1, 1), float textureFactor = 1.0f);

	bool LoadShaders();
    void CreateSpriteVerts();

	// Basic shader
	class Shader* mBaseShader = nullptr;

	// Chess Shader
	class Shader* mChessShader = nullptr;

    // Sprite vertex array
    class VertexArray * mSpriteVerts = nullptr;

	// Window
	SDL_Window* mWindow = nullptr;

	// Ortho projection for 2D shaders
	glm::mat4 mOrthoProjection{};

    // Map of textures loaded
    std::unordered_map<std::string, class Texture*> mTextures;
};