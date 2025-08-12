cmake_minimum_required(VERSION 3.15...4.0)

project(VoxelEngine VERSION 0.1 DESCRIPTION "Port of my Rust OpenGL Voxel Engine to C++ with Vulkan" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME})

# set_target_properties(${PROJECT_NAME} PROPERTIES
#   CXX_STANDARD 23
#   CXX_STANDARD_REQUIRED ON
# )

target_include_directories(${PROJECT_NAME} PRIVATE src)

add_subdirectory(src)

include(FetchContent)

add_subdirectory(Engine)
target_link_libraries(${PROJECT_NAME} PRIVATE VulkanEngine)
target_include_directories(${PROJECT_NAME} PRIVATE VulkanEngine)

include(cmake/vulkan.cmake)

include(cmake/shaders.cmake)
add_subdirectory(shaders)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd5050)
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token)
endif()

add_compile_definitions(CMAKE_PROJECT_DIR=${CMAKE_PROJECT_DIR})
