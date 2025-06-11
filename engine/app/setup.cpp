#include "app.hpp"

#include "logger.hpp"

#include <vk/device/physical.hpp>
#include <vk/pipeline/pipeline.hpp>
#include <vk/structs/attachmentDescription.hpp>
#include <vk/structs/rect2d.hpp>
#include <vk/structs/subpassDependency.hpp>
#include <vk/structs/subpassDescription.hpp>
#include <vk/structs/viewport.hpp>

#include <engine/physical-device-selector.hpp>

#include <array>
#include <optional>
#include <vector>

#include "shaders/shader.hpp"
#include "vertex.hpp"
#include <optional>
#include <span>
#include <vulkan/vulkan_core.h>

const uint32_t WIDTH = 800, HEIGHT = 600;

auto findDevice(vk::Instance &instance, vk::khr::Surface &surface)
    -> std::optional<vk::PhysicalDevice> {
  engine::PhysicalDeviceSelector selector(instance);

  selector.requireExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  selector.canPresentTo(surface);

  if (!selector.hasDevice()) {
    Logger::critical("No suitable physical device found.");
    return std::nullopt;
  }

  auto ratings =
      selector.rateDevices([](vk::PhysicalDevice &device) -> uint32_t {
        uint32_t rating = 0;
        if (device.isDiscrete()) {
          rating += 1000;
        }
        return rating;
      });

  vk::PhysicalDevice &physicalDevice = ratings[0].device;

  Logger::info("Found {} device: {}",
               physicalDevice.isDiscrete() ? "discrete" : "integrated",
               physicalDevice.getProperties().deviceName);

  std::optional<vk::PhysicalDevice> dev = std::move(physicalDevice);

  return dev;
}

auto chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats)
    -> VkSurfaceFormatKHR {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

auto pickPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
    -> VkPresentModeKHR {
  for (const auto &mode : availablePresentModes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

auto createRenderPass(vk::Device &device, vk::khr::Swapchain &swapChain)
    -> std::optional<vk::RenderPass> {
  vk::AttachmentDescription colorAttachment(swapChain.getFormat());

  colorAttachment.setColorDepth(VK_ATTACHMENT_LOAD_OP_CLEAR,
                                VK_ATTACHMENT_STORE_OP_STORE);

  colorAttachment.setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VkAttachmentReference colorAttachmentRef = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  vk::SubpassDescription subpassDescription;
  subpassDescription.color(colorAttachmentRef);

  vk::info::RenderPassCreate renderPassCreateInfo;
  renderPassCreateInfo.addAttachment(colorAttachment);
  renderPassCreateInfo.addSubpass(subpassDescription);

  vk::SubpassDependency subpassDependency{
      {.srcSubpass = VK_SUBPASS_EXTERNAL,
       .dstSubpass = 0,
       .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
       .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
       .srcAccessMask = 0,
       .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
       .dependencyFlags = 0}};

  renderPassCreateInfo.addDependency(subpassDependency);

  auto rp = vk::RenderPass::create(device, renderPassCreateInfo);

  return rp;
}

auto makeShader(vk::Device &device, const char *path, vk::ShaderStage stage)
    -> std::optional<vk::Shader> {
  auto shader = vk::Shader::fromFile(path, stage, device);
  if (!shader.has_value()) {
    Logger::error("Failed to create shader.");
    return std::nullopt;
  }

  return shader;
}

auto makePipeline(vk::Device &device, vk::khr::Swapchain &swapchain,
                  vk::PipelineLayout &layout, vk::RenderPass &renderPass)
    -> std::optional<vk::GraphicsPipeline> {
  vk::info::PipelineDynamicStateCreate dynState;
  dynState.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  dynState.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

  vk::info::PipelineInputAssemblyStateCreate inputAssembly;
  inputAssembly.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

  vk::info::PipelineVertexInputStateCreate vertexInput;
  vertexInput.addBindingDescription(Vertex::getBindingDescription());
  {
    auto attrs = Vertex::getAttributeDescriptions();
    vertexInput.addAttributeDescriptions(attrs);
  }

  vk::info::PipelineRasterizationStateCreate rasterizationState;
  rasterizationState.setPolygonMode(VK_POLYGON_MODE_FILL);
  rasterizationState.setCullMode(VK_CULL_MODE_NONE,
                                 VK_FRONT_FACE_COUNTER_CLOCKWISE);

  vk::info::PipelineMultisampleStateCreate multisampleState;

  vk::info::PipelineColorBlendStateCreate colorBlendState;

  vk::PipelineColorBlendAttachmentState colorBlendAttachment;

  colorBlendState.addAttachment(colorBlendAttachment);

  vk::Viewport viewport(swapchain.getExtent());

  vk::Rect2D scissor(swapchain.getExtent());

  vk::info::PipelineViewportStateCreate viewportState;

  viewportState.addViewport(viewport);
  viewportState.addScissor(scissor);

  auto vertexShader = makeShader(device, "shaders/build/basic.vert.spv",
                                 vk::ShaderStage::Vertex);

  if (!vertexShader.has_value()) {
    Logger::error("Failed to create vertex shader.");
    return std::nullopt;
  }

  auto fragmentShader = makeShader(device, "shaders/build/basic.frag.spv",
                                   vk::ShaderStage::Fragment);

  if (!fragmentShader.has_value()) {
    Logger::error("Failed to create fragment shader.");
    return std::nullopt;
  }

  vk::info::PipelineShaderStageCreate vertexShaderStage(*vertexShader);
  vk::info::PipelineShaderStageCreate fragmentShaderStage(*fragmentShader);

  std::array<vk::info::PipelineShaderStageCreate, 2> shaderStages = {
      vertexShaderStage, fragmentShaderStage};

  std::span<vk::info::PipelineShaderStageCreate> shaderStagesSpan(shaderStages);

  vk::info::GraphicsPipelineCreate pipelineCreateInfo(
      layout, renderPass, shaderStagesSpan, vertexInput, inputAssembly,
      viewportState, rasterizationState, multisampleState, colorBlendState,
      dynState, 0);

  auto pipeline = vk::GraphicsPipeline::create(device, pipelineCreateInfo);

  if (!pipeline.has_value()) {
    Logger::error("Failed to create graphics pipeline.");
    return std::nullopt;
  }

  return pipeline;
}

auto createVertexBuffers(vk::Device &device, vk::Queue &transferQueue)
    -> std::optional<VBufferParts> {
  using VertexT = Vertex;
  using IndexT = uint16_t;
  const int VERTEX_COUNT = 4;
  const int INDEX_COUNT = 6;

  VkDeviceSize bufSize = VERTEX_COUNT * sizeof(VertexT);

  vk::info::VertexBufferCreate vBufInfo(vk::Size(bufSize),
                                        vk::enums::BufferUsage::TransferDst);

  auto vBuf_opt = device.createVertexBuffer(vBufInfo);
  if (!vBuf_opt.has_value()) {
    Logger::error("Failed to create vertex buffer");
    return std::nullopt;
  }
  auto &vBuf = vBuf_opt.value();
  Logger::trace("Create Vertex Buffer");

  VkDeviceSize indexBufSize = INDEX_COUNT * sizeof(IndexT);

  vk::info::IndexBufferCreate indexBufInfo(vk::Size(indexBufSize),
                                           vk::enums::BufferUsage::TransferDst);
  auto indexBuf_opt =
      device.createIndexBuffer(indexBufInfo, vk::enums::IndexType::U16);
  if (!indexBuf_opt.has_value()) {
    Logger::error("Failed to create index buffer");
    return std::nullopt;
  }
  auto &indexBuf = indexBuf_opt.value();
  Logger::trace("Create Index Buffer");

  std::array<vk::Buffer *, 2> buffers = {&vBuf, &indexBuf};
  std::span<vk::Buffer *> buffersSpan(buffers);

  auto memory_opt =
      device.allocateMemory(buffersSpan, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!memory_opt.has_value()) {
    Logger::error("Failed to allocate vertex buffer memory");
    return std::nullopt;
  }
  auto &memory = memory_opt.value();
  Logger::trace("Allocated vertex buffer memory");

  auto vBufBindRes = vBuf.bind(memory);
  if (vBufBindRes.has_value()) {
    Logger::error("Failed to bind vertex buffer memory: {}",
                  vBufBindRes.value());
    return std::nullopt;
  }

  auto indexBufBindRes = indexBuf.bind(memory, vk::Offset(vBuf.size()), true);
  if (indexBufBindRes.has_value()) {
    Logger::error("Failed to bind index buffer memory: {}",
                  indexBufBindRes.value());
    return std::nullopt;
  }
  Logger::trace("Bound vertex and index buffers to memory");

  vk::info::CommandPoolCreate cmdPoolInfo(transferQueue.getFamilyIndex(), false,
                                          true);
  auto cmdPool_opt = device.createCommandPool(cmdPoolInfo);
  if (!cmdPool_opt.has_value()) {
    Logger::error("Failed to create command pool");
    return std::nullopt;
  }
  auto &cmdPool = cmdPool_opt.value();

  auto cmdBuf_opt = cmdPool.allocBuffer();
  if (!cmdBuf_opt.has_value()) {
    Logger::error("Failed to allocate command buffer");
    return std::nullopt;
  }
  auto &cmdBuf = cmdBuf_opt.value();

  Logger::trace("Allocated command buffer");

  vk::info::CommandBufferBegin cmdBufBegin{};
  cmdBufBegin.oneTime();

  std::vector<vk::CommandBuffer::Encoder::TemporaryStaging> writes{};
  writes.reserve(2);

  {
    auto encoder = cmdBuf.begin(cmdBufBegin);

    {
      std::array<VertexT, VERTEX_COUNT> vertices = {
          {{.position = {-0.5f, -0.5f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
           {.position = {0.5f, -0.5f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
           {.position = {0.5f, 0.5f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}},
           {.position = {-0.5f, 0.5f, 0.0f}, .color = {1.0f, 1.0f, 1.0f}}}};
      std::span<VertexT> verticesSpan(vertices);

      auto write = encoder.writeBufferWithStaging(verticesSpan, vBuf);
      if (!write.has_value()) {
        Logger::error("Failed to write buffer with staging");
        return std::nullopt;
      }
      writes.push_back(std::move(write.value()));
    }
    {
      std::array<IndexT, INDEX_COUNT> indices = {{0, 1, 2, 0, 2, 3}};
      std::span<IndexT> indicesSpan(indices);
      auto write = encoder.writeBufferWithStaging(indicesSpan, indexBuf);
      if (!write.has_value()) {
        Logger::error("Failed to write index buffer with staging");
        return std::nullopt;
      }
      writes.push_back(std::move(write.value()));
    }
  }

  auto fence_opt = device.createFence();
  if (!fence_opt.has_value()) {
    Logger::error("Failed to create fence");
    return std::nullopt;
  }
  auto &fence = fence_opt.value();

  transferQueue.submit(cmdBuf, &fence);
  fence.wait();

  VBufferParts parts{
      .vertexBuffer = std::move(vBuf),
      .indexBuffer = std::move(indexBuf),
      .memory = std::move(memory),
  };

  return parts;
}

struct UniformReturn {
  vk::PipelineLayout layout;
  vk::DescriptorPool pool;
  UObjects uniforms;
};

auto setupUniforms(vk::Device &device) -> std::optional<UniformReturn> {
  vk::DescriptorSetLayoutBinding cameraBinding(
      0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);

  vk::info::DescriptorSetLayoutCreate descriptorSetLayoutCreateInfo;
  descriptorSetLayoutCreateInfo.addBinding(cameraBinding);

  auto descriptorSetLayout_opt =
      device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
  if (!descriptorSetLayout_opt.has_value()) {
    Logger::error("Failed to create descriptor set layout.");
    return std::nullopt;
  }
  auto &descriptorSetLayout = descriptorSetLayout_opt.value();
  Logger::trace("Created set layout");

  vk::info::PipelineLayoutCreate pipelineLayoutCreateInfo;
  pipelineLayoutCreateInfo.addSetLayout(descriptorSetLayout);
  auto layout_opt =
      vk::PipelineLayout::create(device, pipelineLayoutCreateInfo);
  if (!layout_opt.has_value()) {
    Logger::error("Failed to create pipeline layout.");
    return std::nullopt;
  }

  vk::info::DescriptorPoolCreate descriptorPoolInfo(MAX_FRAMES_IN_FLIGHT);
  vk::DescriptorPoolSize poolSize(vk::enums::DescriptorType::UniformBuffer,
                                  MAX_FRAMES_IN_FLIGHT);
  descriptorPoolInfo.addPoolSize(poolSize);
  auto descriptorPool_opt = device.createDescriptorPool(descriptorPoolInfo);
  if (!descriptorPool_opt.has_value()) {
    Logger::error("Failed to create descriptor pool.");
    return std::nullopt;
  }
  auto &descriptorPool = descriptorPool_opt.value();
  auto descriptorSets_opt = descriptorPool.allocateSets(descriptorSetLayout, 2);
  if (!descriptorSets_opt.has_value()) {
    Logger::error("Failed to allocate descriptor set.");
    return std::nullopt;
  }
  auto &descriptorSets = descriptorSets_opt.value();
  Logger::debug("Allocated {} descriptor sets", descriptorSets.size());

  const uint32_t UNIFORM_BUFFER_SIZE = 3 * sizeof(glm::mat4);

  vk::info::UniformBufferCreate bufferInfo((vk::Size(UNIFORM_BUFFER_SIZE)));

  std::vector<vk::UniformBuffer> buffers;
  buffers.reserve(MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto uniformBuffer_opt = device.createUniformBuffer(bufferInfo);
    if (!uniformBuffer_opt.has_value()) {
      Logger::error("Failed to create uniform buffer.");
      return std::nullopt;
    }
    buffers.push_back(std::move(uniformBuffer_opt.value()));
  }
  Logger::debug("Created uniform buffers");

  std::array<vk::Buffer *, MAX_FRAMES_IN_FLIGHT> buffersRef = {&buffers[0],
                                                               &buffers[1]};

  auto memory_opt = device.allocateMemory(
      buffersRef, vk::enums::MemoryProperties::HostVisible |
                      vk::enums::MemoryProperties::HostCoherent | 99);
  return std::nullopt;
  if (!memory_opt.has_value()) {
    Logger::error("Failed to allocate uniform buffer memory.");
    return std::nullopt;
  }
  auto &memory = memory_opt.value();
  Logger::trace("Allocated uniform buffer memory");

  for (vk::DeviceSize i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    auto bindRes = buffers[i].bind(memory, vk::Offset(i * UNIFORM_BUFFER_SIZE));
    if (bindRes.has_value()) {
      Logger::error("Failed to bind uniform buffer memory for buffer {}: {}", i,
                    bindRes.value());
      return std::nullopt;
    }
  }
  Logger::trace("Bound uniform buffer memory");

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vk::info::DescriptorBuffer bufInfo(buffers[i]);
    vk::DescriptorSetWriteBuffer write(descriptorSets[i], 0);
    write.addBuffer(bufInfo);
    descriptorSets[i].update(write);
  }
  Logger::trace("Updated descriptor sets with uniform buffers");

  auto mapping_opt = memory.map();
  if (!mapping_opt.has_value()) {
    Logger::error("Failed to map uniform buffer memory.");
    return std::nullopt;
  }
  auto &mapping = mapping_opt.value();

  Logger::trace("Mapped uniform buffer memory");

  std::vector<UObject> uObjs;
  uObjs.reserve(MAX_FRAMES_IN_FLIGHT);

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    UObject uObj{
        .buffer = std::move(buffers[i]),
        .bufferMapping =
            vk::MappingSegment(mapping, vk::Offset(i * UNIFORM_BUFFER_SIZE),
                               vk::Size(UNIFORM_BUFFER_SIZE)),
        .descriptorSet = std::move(descriptorSets[i]),
    };

    uObjs.push_back(std::move(uObj));
  }

  std::array<UObject, MAX_FRAMES_IN_FLIGHT> uniforms{std::move(uObjs[0]),
                                                     std::move(uObjs[1])};

  UObjects uObjects{
      .memory = std::move(memory),
      .mapping = std::move(mapping),
      .objects = std::move(uniforms),
  };

  UniformReturn ret{.layout = std::move(layout_opt.value()),
                    .pool = std::move(descriptorPool),
                    .uniforms = std::move(uObjects)};

  return ret;
}

auto App::create() -> std::optional<App> {
  auto window_opt = vk::Window::create("Vulkan App", WIDTH, HEIGHT, false);
  if (!window_opt.has_value()) {
    Logger::error("Failed to create window.");
    return std::nullopt;
  }
  auto &window = window_opt.value();

  vk::info::Application appInfo;
  vk::info::InstanceCreate createInfo(appInfo);
  auto instance_opt = vk::Instance::create(createInfo);
  if (!instance_opt.has_value()) {
    Logger::error("Failed to create Vulkan instance.");
    return std::nullopt;
  }
  auto &instance = instance_opt.value();

  auto surface_opt = instance.createSurface(window);
  if (!surface_opt.has_value()) {
    Logger::error("Failed to create surface.");
    return std::nullopt;
  }
  auto &surface = surface_opt.value();

  Logger::trace("Window configured");

  auto phys_device_opt = findDevice(instance, surface);
  if (!phys_device_opt.has_value()) {
    Logger::error("Failed to find suitable physical device.");
    return std::nullopt;
  }
  auto &physicalDevice = phys_device_opt.value();

  auto swapchainSupport = vk::khr::SurfaceAttributes(physicalDevice, surface);
  auto format = chooseSwapSurfaceFormat(swapchainSupport.formats);
  auto presentMode = pickPresentMode(swapchainSupport.presentModes);
  auto extent = window.getExtent(swapchainSupport.capabilities);

  auto queueFamilies = physicalDevice.getQueues();
  Logger::info("Found {} queue families", queueFamilies.size());

  auto graphicsQueueFamily_opt = queueFamilies.getGraphics();
  auto presentQueueFamily_opt = queueFamilies.getPresent(surface);

  if (!graphicsQueueFamily_opt.has_value()) {
    Logger::error("Failed to find a graphics queue family.");
    return std::nullopt;
  }

  if (!presentQueueFamily_opt.has_value()) {
    Logger::error("Failed to find a present queue family.");
    return std::nullopt;
  }

  auto graphicsQueueFamily = graphicsQueueFamily_opt.value();
  auto presentQueueFamily = presentQueueFamily_opt.value();

  vk::info::DeviceCreate deviceCreateInfo(physicalDevice.getFeatures());
  deviceCreateInfo.addQueue(graphicsQueueFamily)
      .addQueue(presentQueueFamily)
      .enableExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  Logger::info("Creating device with {} queues",
               deviceCreateInfo.queueCreateInfoCount);

  auto device_opt = vk::Device::create(physicalDevice, deviceCreateInfo);

  if (!device_opt.has_value()) {
    Logger::error("Failed to create logical device.");
    return std::nullopt;
  }
  auto &device = device_opt.value();

  auto graphicsQueue = device.getQueue(graphicsQueueFamily, 0);
  if (!graphicsQueue.has_value()) {
    Logger::error("Failed to create graphics queue.");
    return std::nullopt;
  }

  std::optional<vk::PresentQueue> presentQueue =
      device.getQueue(presentQueueFamily, 0);
  if (!presentQueue.has_value()) {
    Logger::error("Failed to create present queue.");
    return std::nullopt;
  }
  Logger::trace("Created Device and retrieved queues");

  vk::info::SwapchainCreate swapChainCreateInfo(swapchainSupport, surface);
  swapChainCreateInfo.setImageFormat(format.format)
      .setImageColorSpace(format.colorSpace)
      .setImageExtent(extent)
      .setPresentMode(presentMode);

  swapChainCreateInfo.setImageSharingMode(vk::enums::SharingMode::Exclusive)
      .setQueueFamilyIndices(graphicsQueueFamily, presentQueueFamily);

  auto swapchain_opt = device.createSwapchain(swapChainCreateInfo);
  if (!swapchain_opt.has_value()) {
    Logger::error("Failed to create swap chain.");
    return std::nullopt;
  }
  auto &swapchain = swapchain_opt.value();

  auto renderPass_opt = createRenderPass(device, swapchain);
  if (!renderPass_opt.has_value()) {
    Logger::error("Failed to create render pass.");
    return std::nullopt;
  }
  auto &renderPass = renderPass_opt.value();

  auto framebuffers_opt = swapchain.createFramebuffers(renderPass);
  if (!framebuffers_opt.has_value()) {
    Logger::error("Failed to create framebuffers.");
    return std::nullopt;
  }
  auto &framebuffers = framebuffers_opt.value();
  Logger::trace("Swapchain setup");

  auto uniformReturn_opt = setupUniforms(device);
  if (!uniformReturn_opt.has_value()) {
    Logger::error("Failed to setup uniforms.");
    return std::nullopt;
  }

  auto &uniformReturn = uniformReturn_opt.value();
  auto &layout = uniformReturn.layout;

  auto pipeline_opt = makePipeline(device, swapchain, layout, renderPass);
  if (!pipeline_opt.has_value()) {
    Logger::error("Failed to create pipeline.");
    return std::nullopt;
  }
  auto &pipeline = pipeline_opt.value();

  vk::info::CommandPoolCreate commandPoolCreateInfo(graphicsQueueFamily, true);

  auto commandPool_opt = vk::CommandPool::create(device, commandPoolCreateInfo);
  if (!commandPool_opt.has_value()) {
    Logger::error("Failed to create command pool.");
    return std::nullopt;
  }
  auto &commandPool = commandPool_opt.value();

  auto commandBuffer_opt = commandPool.allocBuffers(MAX_FRAMES_IN_FLIGHT);
  if (!commandBuffer_opt.has_value() ||
      commandBuffer_opt.value().size() != MAX_FRAMES_IN_FLIGHT) {
    Logger::error("Failed to allocate command buffer.");
    return std::nullopt;
  }
  auto &commandBuffers = commandBuffer_opt.value();

  auto imageSem1 = device.createSemaphore();
  auto renderSem1 = device.createSemaphore();
  auto inFlightFence1 = device.createFence(true);
  auto imageSem2 = device.createSemaphore();
  auto renderSem2 = device.createSemaphore();
  auto inFlightFence2 = device.createFence(true);

  if (!imageSem1.has_value() || !renderSem1.has_value() ||
      !inFlightFence1.has_value() || !imageSem2.has_value() ||
      !renderSem2.has_value() || !inFlightFence2.has_value()) {
    Logger::error("Failed to create synchronization objects.");
    return std::nullopt;
  }

  std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames{
      {{.commandBuffer = commandBuffers[0],
        .imageAvailable = std::move(imageSem1.value()),
        .inFlight = std::move(inFlightFence1.value())},
       {.commandBuffer = commandBuffers[1],
        .imageAvailable = std::move(imageSem2.value()),
        .inFlight = std::move(inFlightFence2.value())}}};

  auto vBuf_opt = createVertexBuffers(device, graphicsQueue.value());
  if (!vBuf_opt.has_value()) {
    Logger::error("Failed to create vertex buffer.");
    return std::nullopt;
  }

  auto &vBuf = vBuf_opt.value();

  Logger::trace("Creating App");

  App app(window, instance, surface, device, graphicsQueue.value(),
          presentQueue.value(), swapchain, layout, renderPass, framebuffers,
          pipeline, commandPool, frames, vBuf, uniformReturn.pool,
          uniformReturn.uniforms);

  Logger::info("App setup complete");
  return app;
}
