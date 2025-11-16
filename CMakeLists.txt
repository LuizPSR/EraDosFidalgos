cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 17)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "EraDosFidalgos")
project(${PROJECT_NAME})

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(SDL3_IMAGE REQUIRED sdl3-image)
pkg_check_modules(GLEW REQUIRED glew)

include(ImGuiConfig.cmake)
include(FlecsConfig.cmake)

add_executable(${PROJECT_NAME}
        Source/Renderer/Shader.cpp
        Source/Renderer/Shader.h
        Source/Main.cpp
        Source/Math.cpp
        Source/Math.h
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
        ${SDL3_LIBRARIES}
        ${SDL3_IMAGE_LIBRARIES}
        ${GLEW_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDL3_INCLUDE_DIRS}
        ${SDL3_IMAGE_INCLUDE_DIRS}
        ${GLEW_INCLUDE_DIRS})
