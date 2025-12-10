#include "Sound.hpp"
#include <SDL3_mixer/SDL_mixer.h>
#include <filesystem>
#include <string>

// Variáveis estáticas para controle de áudio
namespace {
    MIX_Mixer* mixer = nullptr;
    MIX_Track* ambientTrack = nullptr;
    MIX_Track* gameOverTrack = nullptr;
    MIX_Track* questTrack = nullptr;
    MIX_Audio* ambientAudio = nullptr;
    MIX_Audio* gameOverAudio = nullptr;
    MIX_Audio* questAudio = nullptr;
    bool isGameOverSoundPlaying = false;
}

SoundModule::SoundModule(const flecs::world& ecs)
{
    if (MIX_Init() == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to init mixer: %s", SDL_GetError());
        return;
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (mixer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to setup mixer: %s", SDL_GetError());
        return;
    }

    // Carregar música ambiente
    const auto ambientPath = std::filesystem::path(SDL_GetBasePath()) / "Assets/Medieval Dance - Saltarello [787351].mp3";
    ambientAudio = MIX_LoadAudio(mixer, ambientPath.string().data(), true);

    if (ambientAudio == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load ambient audio: %s", SDL_GetError());
        return;
    }

    ambientTrack = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(ambientTrack, ambientAudio);
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1); // Loop infinito

    if (MIX_PlayTrack(ambientTrack, props) == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play ambient audio: %s", SDL_GetError());
        return;
    }
    SDL_DestroyProperties(props);

    // Carregar som de game over (não tocar ainda)
    const auto gameOverPath = std::filesystem::path(SDL_GetBasePath()) / "Assets/gameOverSound.mp3";
    gameOverAudio = MIX_LoadAudio(mixer, gameOverPath.string().data(), true);

    if (gameOverAudio == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load game over audio: %s", SDL_GetError());
    } else {
        gameOverTrack = MIX_CreateTrack(mixer);
        MIX_SetTrackAudio(gameOverTrack, gameOverAudio);
    }

    // Carregar som de quest
    const auto questPath = std::filesystem::path(SDL_GetBasePath()) / "Assets/Quest.mp3";
    questAudio = MIX_LoadAudio(mixer, questPath.string().data(), true);

    if (questAudio == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load quest audio: %s", SDL_GetError());
    } else {
        questTrack = MIX_CreateTrack(mixer);
        MIX_SetTrackAudio(questTrack, questAudio);
        SDL_Log("Quest sound loaded successfully");
    }

    SDL_Log("Sound module initialized successfully");
}

// Implementação das funções estáticas
void SoundModule::PlayGameOverSound()
{
    if (mixer == nullptr || gameOverTrack == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Cannot play game over sound: mixer not initialized");
        return;
    }

    // Primeiro parar a música ambiente (com fade out de 0 frames = imediato)
    StopAmbientMusic();

    // Tocar som de game over (sem loop)
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, 0); // Não repetir

    if (MIX_PlayTrack(gameOverTrack, props) == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play game over sound: %s", SDL_GetError());
    }
    else
    {
        isGameOverSoundPlaying = true;
        SDL_Log("Game over sound started playing");
    }

    SDL_DestroyProperties(props);
}

void SoundModule::StopAmbientMusic()
{
    if (ambientTrack != nullptr) {
        // Parar a trilha com fade out de 0 frames (imediato)
        MIX_StopTrack(ambientTrack, 0);
        SDL_Log("Ambient music stopped");
    }
}

void SoundModule::PlayAmbientMusic()
{
    if (mixer == nullptr || ambientTrack == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Cannot play ambient music: mixer not initialized");
        return;
    }

    // Primeiro parar o som de game over se estiver tocando
    if (gameOverTrack != nullptr && isGameOverSoundPlaying) {
        MIX_StopTrack(gameOverTrack, 0);
        isGameOverSoundPlaying = false;
    }

    // Tocar música ambiente com loop
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1); // Loop infinito

    if (MIX_PlayTrack(ambientTrack, props) == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play ambient music: %s", SDL_GetError());
    }
    else
    {
        SDL_Log("Ambient music started playing");
    }

    SDL_DestroyProperties(props);
}

void SoundModule::PlayQuestSound()
{
    if (mixer == nullptr || questAudio == nullptr) {
        SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "Cannot play quest sound: mixer or audio not loaded");
        return;
    }

    // Tentativa alternativa: criar um mixer separado para efeitos sonoros
    static MIX_Mixer* effectsMixer = nullptr;

    if (effectsMixer == nullptr) {
        effectsMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
        if (effectsMixer == nullptr) {
            SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to create effects mixer");
            return;
        }
    }

    // Carregar o áudio novamente para o mixer de efeitos
    const auto questPath = std::filesystem::path(SDL_GetBasePath()) / "Assets/Quest.mp3";
    MIX_Audio* effectAudio = MIX_LoadAudio(effectsMixer, questPath.string().data(), true);

    if (effectAudio == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load quest audio for effects mixer");
        return;
    }

    MIX_Track* effectTrack = MIX_CreateTrack(effectsMixer);
    MIX_SetTrackAudio(effectTrack, effectAudio);

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, 0);

    if (MIX_PlayTrack(effectTrack, props)) {
        SDL_Log("Quest sound playing on separate mixer");
    } else {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play quest sound on effects mixer");
    }

    SDL_DestroyProperties(props);
}