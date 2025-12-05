cmake_minimum_required(VERSION 3.26)
# Configurando o padrão c++
set(CMAKE_CXX_STANDARD 20)

cmake_policy(SET CMP0135 NEW)
cmake_policy(SET CMP0003 NEW)

# Configurando o nome e executável do projeto
set(PROJECT_NAME "EraDosFidalgos")
project(${PROJECT_NAME})

set(OpenGL_GL_PREFERENCE LEGACY)
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

# MSVC não define M_PI sem isso
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

target_link_libraries(${PROJECT_NAME} PUBLIC
        ImGui
        flecs::flecs
        SDL3::SDL3
        SDL3_image::SDL3_image
        SDL3_mixer::SDL3_mixer
        GLEW::glew
        tomlplusplus::tomlplusplus
        glm::glm
        m) # Math
# Provides audio for Linux
if (ALSA_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC ALSA::ALSA)
endif ()

# Coloca a pasta Source nos includes
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR})

# Instala Assets e Shaders
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION .
    COMPONENT GameFiles)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Assets/
    DESTINATION Assets
    COMPONENT GameFiles)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/Shaders/
    DESTINATION Shaders
    COMPONENT GameFiles)

# Coloca as DLLs na instalação
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
install(TARGETS flecs LIBRARY DESTINATION . RUNTIME DESTINATION .)
install(TARGETS SDL3-shared LIBRARY DESTINATION . RUNTIME DESTINATION .)
install(TARGETS SDL3_image-shared LIBRARY DESTINATION . RUNTIME DESTINATION .)
install(TARGETS SDL3_mixer-shared LIBRARY DESTINATION . RUNTIME DESTINATION .)
install(TARGETS glew LIBRARY DESTINATION . RUNTIME DESTINATION .)
install(TARGETS glm LIBRARY DESTINATION . RUNTIME DESTINATION .)

if (WIN32)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)
endif ()
