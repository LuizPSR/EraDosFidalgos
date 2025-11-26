include(FetchContent)

FetchContent_Declare(
        flecs_repo
        URL            https://github.com/SanderMertens/flecs/archive/v4.1.2.tar.gz
        URL_HASH       SHA256=9820e965339cca4659dc3b4547059d56889c707e9947e7cdae71847b00dafa9c
)

FetchContent_MakeAvailable(flecs_repo)
