
target_sources(renderer
  PRIVATE
    instance.cpp
    surface.cpp
    window.cpp
    framebuffer.cpp
    swapchain.cpp
    mapping.cpp
)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG master)
FetchContent_MakeAvailable(glm)
target_link_libraries(renderer PUBLIC glm::glm)

add_subdirectory(buffers)
add_subdirectory(pipeline)
add_subdirectory(device)
add_subdirectory(commands)
add_subdirectory(sync)
add_subdirectory(shaders)

target_include_directories(renderer
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(renderer PRIVATE glfw Vulkan::Vulkan)

target_compile_definitions(renderer PUBLIC GLFW_INCLUDE_VULKAN)
