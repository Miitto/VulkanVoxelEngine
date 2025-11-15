

target_sources(${PROJECT_NAME} PRIVATE FILE_SET privateHeaders TYPE HEADERS PRIVATE
  main.cpp
  logger.cpp
  vertex.cpp
  app/app.cpp
  app/setup.cpp
  camera.cpp
)

add_subdirectory(pipelines)
