#pragma once
#include <flecs.h>
#include "Game.hpp"
#include <string>

#include "Systems/Characters.hpp"
#include "Systems/EstatePower.hpp"

struct GameOverModule {
    explicit GameOverModule(flecs::world &ecs);

    static void ShowGameOverScreen(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input);
    static void ShowGameOverScreen(const flecs::world& ecs, GameTickSources& tickSources, const InputState& input,
                                   const std::string& cause, const Character* playerChar = nullptr);

    // Funções para configurar as informações do game over
    static void SetGameOverInfo(const std::string& cause,
                                const std::string& playerName = "",
                                uint64_t playerAgeYears = 0,
                                const std::string& playerTitle = "",
                                const std::string& playerDynasty = "",
                                float playerGold = 0.0f);

    // Função para formatar a causa do game over baseada nos poderes dos estados
    static std::string FormatGameOverCause(const EstatePowers* powers);

    // Função para tocar som do game over
    static void PlayGameOverSound(const flecs::world& ecs);

    // Função para parar som do game over
    static void StopGameOverSound(const flecs::world& ecs);
};
