cmake_minimum_required(VERSION 3.16...4.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(VulkanEngine VERSION 0.1 DESCRIPTION "Vulkan engine" LANGUAGES CXX)

add_library(${PROJECT_NAME})
add_library(vkEngine::vkEngine ALIAS ${PROJECT_NAME})

include(glm)
link_glm(${PROJECT_NAME} PUBLIC)

include(glfw)
link_glfw(${PROJECT_NAME} PUBLIC)

add_subdirectory(vulkan)
add_subdirectory(logger)
add_subdirectory(src)

target_sources(${PROJECT_NAME} PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE logger::logger)
target_link_libraries(${PROJECT_NAME} PUBLIC vkHelpers::vkHelpers)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /wd5050>
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror -Wno-language-extension-token -fno-exceptions>
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

