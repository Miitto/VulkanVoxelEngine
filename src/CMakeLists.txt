target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
  logger.cpp
  vertex.cpp
  app/app.cpp
)

add_subdirectory(pipelines)
