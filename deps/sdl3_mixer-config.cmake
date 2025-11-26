include(FetchContent)

FetchContent_Declare(
        SDL3_mixer
        URL            https://github.com/libsdl-org/SDL_mixer/archive/ff670fa1d43087f2ab496fc9813598995e4dfcdc.tar.gz
        URL_HASH       SHA256=5f6845c66e589d06ae1bb3181760d19f263b5a63137eeb2ac5420a252821d89f
)

FetchContent_MakeAvailable(SDL3_mixer)
