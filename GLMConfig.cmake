include(FetchContent)

FetchContent_Declare(
        glm_repo
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        "1.0.2"
        GIT_SHALLOW    TRUE
        BUILD_AS_SUBDIRECTORY ON
)

FetchContent_MakeAvailable(glm_repo)
