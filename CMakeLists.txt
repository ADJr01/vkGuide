cmake_minimum_required(VERSION 3.31)
project(vkGuide)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure MinGW Makefiles for MSYS2
if (WIN32 AND MSYS)
    set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)
endif()

# Fetch GLFW
include(FetchContent)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4 # Optionally pin to a specific version
)
# Ensure GLFW is built with Vulkan support
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC ON CACHE BOOL "" FORCE) # Link Vulkan statically if desired
FetchContent_MakeAvailable(glfw)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Define executable
add_executable(vkGuide
        src/main.cpp
        src/vulkankit/RenderV.cpp
        src/vulkankit/RenderV.h
        src/vulkankit/RenderVUtil.h
        src/vulkankit/Helper.h
)

# Link libraries and include directories
target_link_libraries(vkGuide PRIVATE glfw Vulkan::Vulkan)
target_include_directories(vkGuide PRIVATE ${Vulkan_INCLUDE_DIRS})

# Optional: Ensure Vulkan SDK is found
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan SDK not found. Please install it from https://www.lunarg.com/vulkan-sdk/")
endif()