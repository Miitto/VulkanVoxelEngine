target_sources(renderer PUBLIC FILE_SET modules TYPE CXX_MODULES BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES
  vk.ixx

  alias.ixx

  buffers/buffer.ixx
  buffers/index.ixx
  buffers/uniform.ixx
  buffers/vertex.ixx

  commands/buffer.ixx
  commands/pool.ixx

  device/device.ixx
  device/memory.ixx
  device/physical.ixx

  pipeline/graphics.ixx
  pipeline/layout.ixx
  pipeline/pipeline.ixx
  pipeline/renderPass.ixx

  shaders/shader.ixx
  shaders/shaderModule.ixx

  structs/attachmentDescription.ixx
  structs/bufferCopy.ixx
  structs/mappedMemoryRange.ixx
  structs/pipelineColorBlendAttachmentState.ixx
  structs/subpassDependency.ixx
  structs/subpassDescription.ixx
  structs/vertexInputAttributeDescription.ixx
  structs/vertexInputBindingDescription.ixx
  structs/viewport.ixx

  structs/info/buffers/create.ixx
  structs/info/commands/bufferAllocate.ixx
  structs/info/commands/bufferBegin.ixx
  structs/info/commands/poolCreate.ixx
  structs/info/commands/renderPassBegin.ixx
  structs/info/pipeline/colorBlendStateCreate.ixx
  structs/info/pipeline/dynamicStateCreate.ixx
  structs/info/pipeline/graphicsPipelineCreate.ixx
  structs/info/pipeline/inputAssemblyStateCreate.ixx
  structs/info/pipeline/layoutCreate.ixx
  structs/info/pipeline/multisampleStateCreate.ixx
  structs/info/pipeline/rasterizationStateCreate.ixx
  structs/info/pipeline/shaderStageCreate.ixx
  structs/info/pipeline/vertexInputStateCreate.ixx
  structs/info/pipeline/viewPortStateCreate.ixx
  structs/info/sync/fenceCreate.ixx
  structs/info/sync/semaphoreCreate.ixx
  structs/info/application.ixx
  structs/info/deviceCreate.ixx
  structs/info/deviceQueueCreate.ixx
  structs/info/framebufferCreate.ixx
  structs/info/imageViewCreate.ixx
  structs/info/instanceCreate.ixx
  structs/info/memoryAllocate.ixx
  structs/info/present.ixx
  structs/info/renderPassCreate.ixx
  structs/info/shaderModuleCreate.ixx
  structs/info/submit.ixx
  structs/info/swapchainCreate.ixx

  sync/fence.ixx
  sync/semaphore.ixx

  instance.ixx
  descriptors.ixx
  framebuffer.ixx
  queue.ixx
  surface.ixx
  swapchain.ixx
  version.ixx
  window.ixx
  PUBLIC

  buffers/buffer.cxx
  buffers/index.cxx
  buffers/uniform.cxx
  buffers/vertex.cxx

  commands/buffer.cxx
  commands/pool.cxx

  device/device.cxx
  device/memory.cxx
  device/physical.cxx

  pipeline/graphics.cxx
  pipeline/layout.cxx
  pipeline/renderPass.cxx

  shaders/shader.cxx
  shaders/shaderModule.cxx

  sync/fence.cxx
  sync/semaphore.cxx

  instance.cxx
  descriptors.cxx
  framebuffer.cxx
  queue.cxx
  surface.cxx
  swapchain.cxx
  window.cxx
)

FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG master)
FetchContent_MakeAvailable(glm)
target_link_libraries(renderer PUBLIC glm::glm)

target_include_directories(renderer
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_link_libraries(renderer PUBLIC glfw Vulkan::Vulkan)

target_compile_definitions(renderer PUBLIC GLFW_INCLUDE_VULKAN)

set_target_properties(renderer
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/renderer/lib"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/renderer/lib"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/out/Debug/renderer/bin"
    ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/renderer/lib"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/renderer/lib"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/out/Release/renderer/bin"
)
