include(FetchContent)

FetchContent_Declare(
        SDL3_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG        ff670fa1d43087f2ab496fc9813598995e4dfcdc
)

FetchContent_MakeAvailable(SDL3_mixer)
