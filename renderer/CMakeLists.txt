
target_sources(renderer
  PRIVATE
    lib.cpp
    shader.cpp
    shaderModule.cpp
    surface.cpp
    window.cpp
    device/physicalDevice.cpp
    device/logicalDevice.cpp
)

target_include_directories(renderer
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/device
)
target_link_libraries(renderer PRIVATE glfw Vulkan::Vulkan)
