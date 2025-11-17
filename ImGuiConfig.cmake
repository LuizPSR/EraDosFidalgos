# CMakeLists.txt for ImGui Dependency Management

# Ensure CMake version 3.14 or newer for FetchContent
cmake_minimum_required(VERSION 3.14)

# Include the module that allows downloading external sources
include(FetchContent)

# ----------------------------------------------------
# 1. FETCH CONTENT (Download ImGui Source)
# ----------------------------------------------------
# Define how to fetch the specific version of ImGui you need.
FetchContent_Declare(
        imgui_repo
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG        v1.92.3
        GIT_SHALLOW    TRUE
)

# Download and make the content available in the build tree
FetchContent_MakeAvailable(imgui_repo)

# ----------------------------------------------------
# 2. DEFINE THE LIBRARY TARGET
# ----------------------------------------------------
# Define the source files using the path where FetchContent placed the code.
set(IMGUI_SOURCES
        "${imgui_repo_SOURCE_DIR}/imgui.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_repo_SOURCE_DIR}/imgui_demo.cpp"
        # Include the required backends for SDL3 and OpenGL/GLEW
        "${imgui_repo_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
        "${imgui_repo_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp")

# Create the library target (defaulting to STATIC library)
add_library(ImGui STATIC ${IMGUI_SOURCES})
target_compile_options(ImGui PRIVATE -fPIC)

# ----------------------------------------------------
# 3. CONFIGURE INCLUDES AND DEPENDENCIES
# ----------------------------------------------------

# Find the dependencies needed for ImGui's backends
find_package(SDL3 REQUIRED)
find_package(GLEW REQUIRED CONFIG)

# Set the public include directories so consuming projects can see ImGui headers
target_include_directories(ImGui PUBLIC
        $<BUILD_INTERFACE:${imgui_repo_SOURCE_DIR}>
        $<BUILD_INTERFACE:${imgui_repo_SOURCE_DIR}/backends>
)

# Link the required libraries to the ImGui target
target_link_libraries(ImGui PRIVATE ${SDL3_LIBRARIES} GLEW::glew)

# Set properties on the target, making it easy to use for the main app
set_target_properties(ImGui PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
)