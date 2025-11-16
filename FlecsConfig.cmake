# Ensure CMake version 3.14 or newer for FetchContent
if (NOT CMAKE_VERSION VERSION_LESS 3.14)
    include(FetchContent)
else()
    message(FATAL_ERROR "CMake 3.14 or newer is required for FetchContent.")
endif()

# ----------------------------------------------------
# 1. FETCH CONTENT (Download Flecs Source)
# ----------------------------------------------------
FetchContent_Declare(
        flecs_repo
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG        v4.1.2
        GIT_SHALLOW    TRUE
        # Important: Set the directory where the source will be placed 
        # and then build the target directly from that directory.
        # We will use the subdirectory option to run Flecs's internal CMakeLists.txt
        BUILD_AS_SUBDIRECTORY ON
)

# Download and make the content available.
# This command automatically runs the CMakeLists.txt inside the Flecs source,
# which defines the library target 'flecs'.
FetchContent_MakeAvailable(flecs_repo)

# --- Define Properties for the Flecs Target ---
# Flecs's CMake file defines a target named 'flecs'. 
# We ensure it has necessary properties set for easy consumption by the main project.
set_target_properties(flecs PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
)