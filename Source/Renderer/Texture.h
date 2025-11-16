#pragma once
#include <string>
#include <GL/glew.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class Texture
{
public:
	Texture();
	~Texture();

	bool Load(const std::string& fileName);
	void Unload() const;

	void SetActive(int index = 0) const ;

    static GLenum SDLFormatToGL(SDL_PixelFormat* fmt);

	[[nodiscard]] int GetWidth() const { return mWidth; }
	[[nodiscard]] int GetHeight() const { return mHeight; }

	[[nodiscard]] unsigned int GetTextureID() const { return mTextureID; }

private:
	unsigned int mTextureID;
	int mWidth;
	int mHeight;
};
