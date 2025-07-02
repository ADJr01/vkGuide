cmake_minimum_required(VERSION 3.16)
project(vkGuide LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLEW REQUIRED IMPORTED_TARGET glew)
pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)
find_package(OpenGL REQUIRED)      # Optional, if using OpenGL
find_package(Vulkan REQUIRED)      # Required for Vulkan

add_executable(vkGuide src/main.cpp
        src/vulkankit/RenderV.cpp
        src/vulkankit/RenderV.h
        src/vulkankit/RenderVUtil.h
        src/vulkankit/Helper.h)

target_link_libraries(vkGuide PRIVATE
        PkgConfig::GLEW
        PkgConfig::GLFW
        OpenGL::GL           # Optional
        Vulkan::Vulkan       # Required
)
