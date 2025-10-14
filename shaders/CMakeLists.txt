include(shaders)

message(STATUS Module Path: ${CMAKE_MODULE_PATH})

compile_shader(shaders SPIRV ${PROJECT_NAME} SOURCES
  basic
  vertex
  INCLUDES
  camera
)

COPY_SHADERS(${PROJECT_NAME} shaders)
