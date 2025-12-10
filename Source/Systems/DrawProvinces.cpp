#include "DrawProvinces.hpp"

#include <SDL3/SDL.h>
#include <GL/glew.h>
#include <flecs.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Characters.hpp"
#include "Diplomacy.hpp"
#include "imgui.h"
#include "Components/Camera.hpp"
#include "Components/Province.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Shader.hpp"

constexpr float TILE_SIZE_WORLD = 32.0f;
glm::vec3 CultureColor(CultureType culture)
{
    switch (culture)
    {
    case SteppeNomads:
        return {0.7f, 0.6f, 0.4f};
    case FarmLanders:
        return {0.4f, 0.8f, 0.3f};
    case ForestFolk:
        return {0.1f, 0.4f, 0.15f};
    case HillDwellers:
        return {0.5f, 0.5f, 0.55f};
    default:
        return {0.0f, 0.0f, 0.0f};
    }
}

void renderPoliticalMap(flecs::iter &it)
{
    while (it.next())
    {
        const auto &renderer = it.field_at<const Renderer>(2, 0);
        const auto &camera = it.field_at<const Camera>(3, 0);
        const auto &window = it.field_at<const Window>(4, 0);

        const auto &shader = renderer.mPoliticalShader;
        const auto &verts = renderer.mSpriteVerts;
        shader->SetActive();
        verts->SetActive();

        const auto &provinces = it.field<const Province>(0);
        const auto &title = it.field_at<const Title>(1, 0);

        for (size_t i: it)
        {
            const auto &province = provinces[i];
            glm::mat4 model = glm::translate(
                            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f) * TILE_SIZE_WORLD),
                            glm::vec3(province.mPosX, province.mPosY, 0.0f));

            shader->SetMatrixUniform("uView", camera.mView);
            shader->SetMatrixUniform("uProj", camera.CalculateProjection(window));
            shader->SetMatrixUniform("uModel", model);
            shader->SetVectorUniform("uRealmColor", glm::vec4(title.color, 1.0));
            shader->SetVectorUniform("uMousePos", window.GetMousePosNDC());

            glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT, 0);
        }
    }
}

void renderCultureMap(flecs::iter &it)
{
    while (it.next())
    {
        const auto &renderer = it.field_at<const Renderer>(2, 0);
        const auto &camera = it.field_at<const Camera>(3, 0);
        const auto &window = it.field_at<const Window>(4, 0);

        const auto &shader = renderer.mPoliticalShader;
        const auto &verts = renderer.mSpriteVerts;
        shader->SetActive();
        verts->SetActive();

        const auto &provinces = it.field<const Province>(0);
        for (size_t i: it)
        {
            const auto &province = provinces[i];
            glm::mat4 model = glm::translate(
                            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f) * TILE_SIZE_WORLD),
                            glm::vec3(province.mPosX, province.mPosY, 0.0f));

            shader->SetMatrixUniform("uView", camera.mView);
            shader->SetMatrixUniform("uProj", camera.CalculateProjection(window));
            shader->SetMatrixUniform("uModel", model);
            shader->SetVectorUniform("uRealmColor", glm::vec4(CultureColor(province.culture), 1.0));
            shader->SetVectorUniform("uMousePos", window.GetMousePosNDC());

            glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT, 0);
        }
    }
}

void renderDiplomacyMap(flecs::iter &it, flecs::entity playerRealm)
{
    while (it.next())
    {
        const auto &renderer = it.field_at<const Renderer>(2, 0);
        const auto &camera = it.field_at<const Camera>(3, 0);
        const auto &window = it.field_at<const Window>(4, 0);

        const auto &shader = renderer.mHeatMapShader;
        const auto &verts = renderer.mSpriteVerts;
        shader->SetActive();
        verts->SetActive();

        const auto &provinces = it.field<const Province>(0);
        for (size_t i: it)
        {
            const auto &province = provinces[i];
            const auto &relation = playerRealm.try_get<RealmRelation>(it.get_var("title"));

            float relationF = relation ? (float(relation->relations) / 256.0f + 0.5f) : 0.5f;
            if (it.get_var("title") == playerRealm) relationF = 1.0f;

            glm::mat4 model = glm::translate(
                            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 0.0f) * TILE_SIZE_WORLD),
                            glm::vec3(province.mPosX, province.mPosY, 0.0f));

            shader->SetMatrixUniform("uView", camera.mView);
            shader->SetMatrixUniform("uProj", camera.CalculateProjection(window));
            shader->SetMatrixUniform("uModel", model);
            shader->SetVectorUniform("uMousePos", window.GetMousePosNDC());
            shader->SetVectorUniform("uMinColor", glm::vec4(1.0, 0.0, 0.0, 1.0));
            shader->SetVectorUniform("uMaxColor", glm::vec4(0.0, 1.0, 0.0, 1.0));
            shader->SetFloatUniform("uPercent", relationF);

            glDrawElements(GL_TRIANGLES, verts->GetNumIndices(), GL_UNSIGNED_INT, 0);
        }
    }
}

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

MapMode mapMode = MapMode::Geographic;

void DoRenderTileMapSystem(const flecs::world &ecs)
{
    auto &renderer = ecs.get_mut<Renderer>();
    auto texturePtr = new Texture();
    texturePtr->Load((std::filesystem::path(SDL_GetBasePath()) / "Assets/Sprites/Tileset.png").string());
    renderer.mTextures.insert({"mapTexture", texturePtr});

    ecs.system<const Province, const Renderer, const Camera, const Window>("RenderMap")
        .kind(flecs::PreStore)
        .run([=](flecs::iter &it)
        {
            RenderTileMap(it);
        });

    flecs::query qPlayerRealm = ecs.query_builder<const Title>("PlayerRealm")
        .with<RulerOf>("$this").src<Player>()
        .without<InRealm>()
        .build();

    ecs.system<const Province, const Title, const Renderer, const Camera, const Window>("RenderMapTypes")
        .term_at(1).src("$title")
        .with<InRealm>("$title")
        .without<InRealm>(flecs::Wildcard).src("$title")
        .kind(flecs::PreStore)
        .run([=](flecs::iter &it)
        {
            switch (mapMode)
            {
            case MapMode::Geographic: break;
            case MapMode::Political: renderPoliticalMap(it); break;
            case MapMode::Cultural: renderCultureMap(it); break;
            case MapMode::Diplomatic: renderDiplomacyMap(it, qPlayerRealm.first()); break;
            }
        });

    ecs.system<>("ChooseMapMode")
        .run([](flecs::iter)
        {
            if (ImGui::Begin("Modo de Mapa", 0, ImGuiWindowFlags_AlwaysAutoResize))
            {
                int mapModeInt = (int)mapMode;
                ImGui::RadioButton("Geográfico", &mapModeInt, (int)MapMode::Geographic);
                ImGui::RadioButton("Politico", &mapModeInt, (int)MapMode::Political);
                ImGui::RadioButton("Cultural", &mapModeInt, (int)MapMode::Cultural);
                ImGui::RadioButton("Diplomático", &mapModeInt, (int)MapMode::Diplomatic);
                mapMode = (MapMode)mapModeInt;
            }
            ImGui::End();
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
