#pragma once
#include <flecs.h>

struct SoundModule {
    explicit SoundModule(const flecs::world& ecs);

    // Funções estáticas para controle de áudio
    static void PlayGameOverSound();
    static void StopAmbientMusic();
    static void PlayAmbientMusic();
    static bool IsGameOverSoundPlaying();
    static void PlayQuestSound();  // Nova função para som de quest
};