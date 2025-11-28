#include "Sound.hpp"

#include <SDL3_mixer/SDL_mixer.h>
#include <filesystem>

SoundModule::SoundModule(const flecs::world& ecs)
{
    if (MIX_Init() == false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed  to init mixer: %s", SDL_GetError());
        return;
    }

    MIX_Mixer *mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (mixer == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to setup mixer: %s", SDL_GetError());
        return;
    }

    const auto songPath = std::filesystem::path(SDL_GetBasePath()) / "Assets/Medieval Dance - Saltarello [787351].mp3";
    const auto audio = MIX_LoadAudio(mixer, songPath.c_str(), true);

    if (audio == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to load audio: %s", SDL_GetError());
        return;
    }

    const auto track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, audio);
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    if (MIX_PlayTrack(track, props) ==  false)
    {
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to play audio: %s", SDL_GetError());
        return;
    }
    SDL_DestroyProperties(props);

    // TODO: Add more songs and options to control the playback ingame
}
