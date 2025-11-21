include(FetchContent)

FetchContent_Declare(
        flecs_repo
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG        v4.1.2
        GIT_SHALLOW    TRUE
        BUILD_AS_SUBDIRECTORY ON
)

FetchContent_MakeAvailable(flecs_repo)
