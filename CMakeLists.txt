cmake_minimum_required(VERSION 3.15...4.0)

project(VoxelEngine VERSION 0.1 DESCRIPTION "Port of my OpenGL Voxel Engine to C++ with Vulkan" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Export for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(engine)
target_include_directories(engine PRIVATE engine)
add_subdirectory(VulkanRenderer)
target_link_libraries(engine PRIVATE VulkanRenderer)

target_sources(engine PRIVATE
  engine/main.cpp
  engine/vertex.cpp
)

target_sources(engine PUBLIC FILE_SET modules BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/engine TYPE CXX_MODULES
  FILES
    engine/app/app.ixx
    engine/app/common.ixx
    engine/app/setup.ixx
    engine/app/class.ixx
)


if(MSVC)
  target_compile_options(engine PRIVATE /W4 /WX)
else()
  target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

if (WIN32)
  add_custom_command(OUTPUT shaders/build COMMAND pwsh ${CMAKE_CURRENT_SOURCE_DIR}/makeShaders.ps1 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
else()
  add_custom_command(OUTPUT shaders/build COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/makeShaders.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
add_custom_target(BuildShaders ALL DEPENDS shaders/build)
