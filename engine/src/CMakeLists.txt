target_sources(${PROJECT_NAME} PRIVATE
  FILE_SET privateHeaders
  TYPE HEADERS
  PRIVATE
  app.cpp
  core.cpp
  image.cpp
  logger.cpp
  window.cpp
  setup.cpp
  debug.cpp
)