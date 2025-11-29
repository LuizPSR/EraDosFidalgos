include(FetchContent)

FetchContent_Declare(
        glew_2_2_0
        URL            https://downloads.sourceforge.net/project/glew/glew/2.2.0/glew-2.2.0.tgz
        URL_HASH       SHA256=d4fc82893cfb00109578d0a1a2337fb8ca335b3ceccf97b97e5cc7f08e4353e1
        SOURCE_SUBDIR  build/cmake
)

FetchContent_MakeAvailable(glew_2_2_0)

if (NOT TARGET GLEW::glew)
    add_library(GLEW::glew ALIAS glew)
endif()