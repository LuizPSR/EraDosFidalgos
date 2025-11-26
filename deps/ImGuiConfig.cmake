include(FetchContent)

FetchContent_Declare(
        imgui_repo
        URL            https://github.com/ocornut/imgui/archive/v1.92.3-docking.tar.gz
        URL_HASH       SHA256=fd0f88fcc593faa4d536c02baded6abc85463b9deb2d75ee829fed6d5dd48e0e
)

FetchContent_MakeAvailable(imgui_repo)

set(IMGUI_SOURCES
        "${imgui_repo_SOURCE_DIR}/imgui.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_demo.cpp"
        # Include the required backends for SDL3 and OpenGL/GLEW
        "${imgui_repo_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
        "${imgui_repo_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")

add_library(ImGui STATIC ${IMGUI_SOURCES})
target_compile_options(ImGui PRIVATE -fPIC)

find_package(SDL3 REQUIRED CONFIG)
find_package(GLEW REQUIRED CONFIG)

target_include_directories(ImGui PUBLIC
        $<BUILD_INTERFACE:${imgui_repo_SOURCE_DIR}>
        $<BUILD_INTERFACE:${imgui_repo_SOURCE_DIR}/backends>
)

target_link_libraries(ImGui PRIVATE SDL3::SDL3 GLEW::glew)
