include(shaders)

message(STATUS Module Path: ${CMAKE_MODULE_PATH})

compile_shader(shaders SPIRV ${PROJECT_NAME} SOURCES
  basic
  mesh
  INCLUDES
  camera
)

COPY_SHADERS(${PROJECT_NAME} shaders)
