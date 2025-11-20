cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "EraDosFidalgos")
project(${PROJECT_NAME})

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(GLEW REQUIRED CONFIG)

include(FlecsConfig.cmake)
include(GLMConfig.cmake)
include(ImGuiConfig.cmake)
include(TOMLConfig.cmake)

set(ASSETS_DIR "${PROJECT_SOURCE_DIR}/Assets")
set(ASSET_DESTINATION_DIR "${CMAKE_CURRENT_BINARY_DIR}")
file(COPY "${ASSETS_DIR}" DESTINATION "${ASSET_DESTINATION_DIR}")

add_executable(${PROJECT_NAME}
        Source/Renderer/Shader.cpp
        Source/Renderer/Shader.h
        Source/Main.cpp
        Source/Random.cpp
        Source/Random.h
        Source/Game.cpp
        Source/Game.h
        Source/Renderer/Renderer.cpp
        Source/Renderer/Renderer.h
        Source/Renderer/VertexArray.h
        Source/Renderer/VertexArray.cpp
        Source/Renderer/Texture.cpp
        Source/Renderer/Texture.h
        Source/Entities/MapTile.h
        Source/Components/AnimatorComponent.cpp
        Source/Components/AnimatorComponent.h
        Source/Components/RectComponent.cpp
        Source/Components/RectComponent.h
        Source/Entities/Settlement.h
        Source/Entities/Character.h
        Source/Entities/Army.h
        Source/Entities/Event.h
        Source/Components/Camera.cpp
        Source/Components/Camera.h
        Source/Systems/ChessBoard.cpp
        Source/Systems/ChessBoard.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        ImGui
        flecs::flecs
        SDL3::SDL3
        SDL3_image::SDL3_image
        GLEW::glew
        tomlplusplus::tomlplusplus
        glm::glm
        m) # Math