cmake_minimum_required(VERSION 3.15...4.0)

project(VulkanEngine VERSION 0.1 DESCRIPTION "Vulkan engine helpers" LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PUBLIC
  engine/core.cpp
  engine/logger.cpp
  engine/image.cpp
  engine/window.cpp
  engine/vulkan/vk-logger.cpp
  engine/vulkan/extensions/swapchain.cpp
  engine/vulkan/extensions/shader.cpp
  engine/vulkan/queueFinder.cpp
)

target_include_directories(${PROJECT_NAME} INTERFACE
  "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
  "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/vulkan.cmake)
find_vulkan()
link_vulkan(${PROJECT_NAME})

include(cmake/glm.cmake)
link_glm(${PROJECT_NAME})

include(cmake/glfw.cmake)
link_glfw(${PROJECT_NAME})

include(cmake/spdlog.cmake)
link_spdlog_header_only(${PROJECT_NAME})


target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd5050>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token>
)

target_precompile_headers(${PROJECT_NAME} PUBLIC
  <vector>
  <array>
  <span>
  <expected>
  <optional>
  <string>
  <algorithm>
  <memory>
  <utility>
)
