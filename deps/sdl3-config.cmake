include(FetchContent)

FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        6a1218c1cc9fe716089dcf9b25754854be9292c8
)

FetchContent_MakeAvailable(SDL3)
