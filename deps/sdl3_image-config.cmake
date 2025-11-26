include(FetchContent)

FetchContent_Declare(
        SDL3_image
        URL         https://github.com/libsdl-org/SDL_image/archive/release-3.2.4.tar.gz
        URL_HASH    SHA256=6a2f263e69d5cf0f218615e061f0d6cc1ae708e847ffe5372af98d5dd96bd037
)

FetchContent_MakeAvailable(SDL3_image)
