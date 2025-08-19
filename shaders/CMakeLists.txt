include(cmake/shaders.cmake)

compile_shader(shaders SOURCES
  basic
  vertex
  INCLUDES
  camera
)

add_dependencies(${PROJECT_NAME} shaders)

target_compile_definitions(VulkanEngine PUBLIC SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/build/")
