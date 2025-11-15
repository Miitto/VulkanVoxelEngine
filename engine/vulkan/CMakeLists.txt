cmake_minimum_required(VERSION 3.16...4.0)

project(VkHelpers VERSION 0.1 DESCRIPTION "Vulkan helpers" LANGUAGES CXX)

add_library(${PROJECT_NAME})
add_library(vkHelpers::vkHelpers ALIAS ${PROJECT_NAME})

include(vulkan)
find_vulkan()
LINK_VULKAN(${PROJECT_NAME} PUBLIC)

target_link_libraries(${PROJECT_NAME} PRIVATE logger::logger glfw)

add_subdirectory(src)
target_sources(${PROJECT_NAME} PUBLIC
  FILE_SET HEADERS
  BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include"
)