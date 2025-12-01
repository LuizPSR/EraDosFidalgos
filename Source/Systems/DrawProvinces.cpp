#include "DrawProvinces.hpp"

#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <flecs.h>

#include "Characters.hpp"
#include "Components/Camera.hpp"
#include "Components/Province.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Shader.hpp"

constexpr float TILE_SIZE_WORLD = 32.0f;

void RenderTileMap(flecs::iter &it)
{
    while (it.next())
    {
        const auto &renderer = it.field_at<const Renderer>(1, 0);
        const auto &camera = it.field_at<const Camera>(2, 0);
        const auto &window = it.field_at<const Window>(3, 0);

        renderer.mTextures.at("mapTexture")->SetActive();
        const auto &shader = renderer.mMapShader;
        const auto &verts = renderer.mSpriteVerts;
        shader->SetActive();
        verts->SetActive();

        const auto &provinces = it.field<const Province>(0);
        for (size_t i: it)
        {
            const auto &province = provinces[i];

            int64_t textureIndex = 0;
            switch (province.biome)
            {
                case Water: textureIndex = 10; break;
                case Drylands: textureIndex = 1; break;
                case Grasslands: textureIndex = 3; break;
                case Jungles: textureIndex = 5; break;
                case Forests: textureIndex = 2; break;
            }

            glm::mat4 model = glm::translate(
                glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f) * TILE_SIZE_WORLD),
                glm::vec3(province.mPosX, province.mPosY, 0.0f));

            shader->SetMatrixUniform("uView", camera.mView);
            shader->SetMatrixUniform("uProj", camera.CalculateProjection(window));
            shader->SetMatrixUniform("uModel", model);
            shader->SetIntegerUniform("uTileIndex", textureIndex);
            shader->SetVectorUniform("uMousePos", window.GetMousePosNDC());

            glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT, 0);

            switch (province.terrain)
            {
                case Sea: textureIndex = -1; break;
                case Wetlands: textureIndex = 11; break;
                case Plains: textureIndex = -1; break;
                case Hills: textureIndex = 4; break;
                case Mountains: textureIndex = 6; break;
            }
            if (textureIndex != -1)
            {
                shader->SetIntegerUniform("uTileIndex", textureIndex);

                glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT, 0);
            }

            if (it.entity(i).has<CapitalOf>(flecs::Wildcard))
            {
                shader->SetIntegerUniform("uTileIndex", 0);

                glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT, 0);
            }
        }
    }

    // Unbind
    glBindVertexArray(0);
}

void DoRenderTileMapSystem(const flecs::world &ecs)
{
    auto &renderer = ecs.get_mut<Renderer>();
    auto texturePtr = new Texture();
    texturePtr->Load(std::filesystem::path(SDL_GetBasePath()) / "Assets/Sprites/Tileset.png");
    renderer.mTextures.insert({"mapTexture", texturePtr});

    ecs.system<const Province, const Renderer, const Camera, const Window>("RenderMap")
        .kind(flecs::PreStore)
        .run([=](flecs::iter &it)
        {
            RenderTileMap(it);
        });

    ecs.system<const Province, const Camera, const Window>("SetHoveredProvince")
        .kind(flecs::PreUpdate)
        .each([](flecs::entity entity, const Province &province, const Camera &camera, const Window &window)
        {
            const auto &mousePosNDC = window.GetMousePosNDC();
            const auto &mousePosWorld = camera.NDCToWorld(mousePosNDC, window);
            const auto mousedOverTile = glm::round(glm::vec2(mousePosWorld) / TILE_SIZE_WORLD);
            const glm::vec2 provinceTile(province.mPosX, province.mPosY);
            if (mousedOverTile == provinceTile)
                void(entity.add<Hovered>());
            else
                void(entity.remove<Hovered>());
        });
}
