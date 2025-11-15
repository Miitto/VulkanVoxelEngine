target_sources(${PROJECT_NAME} PRIVATE
  FILE_SET privateHeaders
  TYPE HEADERS
  PRIVATE
  queueFinder.cpp
  shader.cpp
  swapchain.cpp
  vk-logger.cpp
  instance.cpp
  validators.cpp
  physicalDevice.cpp
)