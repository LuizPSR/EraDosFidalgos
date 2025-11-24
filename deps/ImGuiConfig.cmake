include(FetchContent)

FetchContent_Declare(
        imgui_repo
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.92.3-docking
        GIT_SHALLOW    TRUE
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
