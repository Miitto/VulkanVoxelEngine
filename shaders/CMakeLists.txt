compile_shader(shaders SOURCES
  basic
)

add_dependencies(${PROJECT_NAME} shaders)

target_compile_definitions(${PROJECT_NAME} PUBLIC SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/build/")