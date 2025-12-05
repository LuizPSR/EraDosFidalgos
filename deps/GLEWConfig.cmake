if (NOT TARGET GLEW::glew)
    include(FetchContent)
    
    FetchContent_Declare(
            glew_repo
            URL            https://downloads.sourceforge.net/project/glew/glew/2.2.0/glew-2.2.0.tgz
            URL_HASH       SHA256=d4fc82893cfb00109578d0a1a2337fb8ca335b3ceccf97b97e5cc7f08e4353e1
            EXCLUDE_FROM_ALL)
    
    FetchContent_MakeAvailable(glew_repo)
    
    set(GLEW_SOURCES "${glew_repo_SOURCE_DIR}/src/glew.c")
    
    add_library(glew SHARED ${GLEW_SOURCES})
    target_compile_options(glew PRIVATE -fPIC)
    target_compile_definitions(glew PRIVATE GLEW_NO_GLU GLEW_BUILD)
    target_link_options(glew PRIVATE -nostdlib)
    
    target_include_directories(glew PUBLIC ${glew_repo_SOURCE_DIR}/include)

    find_package(OpenGL REQUIRED)
    target_link_libraries(glew PUBLIC OpenGL::GL)
    
    add_library(GLEW::glew ALIAS glew)
endif ()
