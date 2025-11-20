#include <SDL3_image/SDL_image.h>
#include "Texture.h"

Texture::Texture()
: mTextureID(0)
, mWidth(0)
, mHeight(0)
{
}

Texture::~Texture()
{
}

bool Texture::Load(const std::string &filePath)
{
    // Load texture
    SDL_Surface* surf = IMG_Load(filePath.c_str());
    if (!surf) {
        SDL_Log("Failed to load texture file %s", filePath.c_str());
        return false;
    }

    if (surf->format != SDL_PIXELFORMAT_RGBA32)
    {
        SDL_Surface *converted = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
        if (!converted)
        {
            SDL_Log("Failed to convert texture file %s", filePath.c_str());
            return false;
        }
        SDL_DestroySurface(surf);
        surf = converted;
    }

    // Load to GPU
    GLuint textureID;
    glGenTextures(1, &textureID); // Cria textura na GPU
    glActiveTexture(GL_TEXTURE0 + textureID); // Coloca textura na memória de textura
    glBindTexture(GL_TEXTURE_2D, textureID); // Ativa a textura no pipeline gráfico
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);

    // Configura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    mTextureID = textureID;
    mWidth = surf->w;
    mHeight = surf->h;


    return true;
}

void Texture::Unload() const {
	glDeleteTextures(1, &mTextureID);
}

void Texture::SetActive(const int index) const
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
}
