cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 20)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "EraDosFidalgos")
project(${PROJECT_NAME})

list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/deps")

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_mixer REQUIRED CONFIG)
find_package(GLEW REQUIRED CONFIG)
find_package(Flecs REQUIRED CONFIG)
find_package(GLM REQUIRED CONFIG)
find_package(ImGui REQUIRED CONFIG)
find_package(TOML REQUIRED CONFIG)
find_package(ALSA QUIET)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "Source/*.hpp")
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "Source/*.cpp")
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
        ImGui
        flecs::flecs
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        GLEW::glew
        tomlplusplus::tomlplusplus
        glm::glm
        m) # Math
if (ALSA_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ALSA::ALSA)
endif ()

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR})