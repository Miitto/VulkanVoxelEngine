cmake_minimum_required(VERSION 3.15...4.0)

project(VoxelEngine VERSION 0.1 DESCRIPTION "Port of my OpenGL Voxel Engine to C++ with Vulkan" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Export for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

add_executable(engine)
target_include_directories(engine PRIVATE engine)
add_subdirectory(engine)

target_link_libraries(engine PUBLIC renderer)


add_library(renderer)
add_subdirectory(renderer)

if(MSVC)
  target_compile_options(engine PRIVATE /W4 /WE)
  target_compile_options(renderer PRIVATE /W4 /WE)
else()
  target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic -Werror)
  target_compile_options(renderer PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if (WIN32)
  add_custom_command(OUTPUT shaders/build COMMAND pwsh ${CMAKE_CURRENT_SOURCE_DIR}/makeShaders.ps1 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  add_custom_command(OUTPUT shaders/build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/makeShaders.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
add_custom_target(BuildShaders ALL DEPENDS shaders/build)
