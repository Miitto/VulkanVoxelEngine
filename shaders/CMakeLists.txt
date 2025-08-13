compile_shader(shaders SOURCES
  basic
)

add_dependencies(${PROJECT_NAME} shaders)
