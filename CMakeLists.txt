cmake_minimum_required(VERSION 3.15...4.0)

project(VoxelEngine VERSION 0.1 DESCRIPTION "Port of my OpenGL Voxel Engine to C++ with Vulkan" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS OFF)
# Export for clang
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

add_executable(engine "engine/main.cpp")
target_include_directories(engine PRIVATE engine)
target_include_directories(engine PRIVATE .)

add_library(renderer STATIC "renderer/lib.cpp" "renderer/window.cpp" "renderer/queue.cpp" "renderer/logicalDevice.cpp" "renderer/physicalDevice.cpp" "renderer/surface.cpp" "renderer/swapChain.cpp")


target_link_libraries(engine PUBLIC renderer)
target_link_libraries(engine PRIVATE glfw Vulkan::Vulkan)
target_link_libraries(renderer PRIVATE glfw Vulkan::Vulkan)
