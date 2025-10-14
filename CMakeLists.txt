cmake_minimum_required(VERSION 3.15...4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/engine/cmake")

project(VoxelEngine VERSION 0.1 DESCRIPTION "Port of my Rust OpenGL Voxel Engine to C++ with Vulkan" LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_program(SCCACHE_FOUND sccache)

if(SCCACHE_FOUND)
  message(STATUS "Using sccache for compilation")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${SCCACHE_FOUND})
  set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_FOUND})

  # For MSVC debug information format
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT Embedded)
  cmake_policy(SET CMP0141 NEW)
else()
  message(STATUS "sccache not found, using default compiler")
endif()

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE src)

add_subdirectory(src)

include(FetchContent)

add_subdirectory(engine)
target_link_libraries(${PROJECT_NAME} PRIVATE VulkanEngine)
target_include_directories(${PROJECT_NAME} PRIVATE VulkanEngine)

add_subdirectory(shaders)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd5050>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token -fno-exceptions>
)

add_compile_definitions(CMAKE_PROJECT_DIR=${CMAKE_PROJECT_DIR})
