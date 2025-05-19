#include "app.h"
#include "commands/buffer.h"
#include "instance.h"
#include "log.h"
#include "structs/attachmentDescription.h"
#include "structs/info/all.h"
#include "structs/pipelineColorBlendAttachmentState.h"
#include "structs/subpassDependency.h"
#include "structs/subpassDescription.h"
#include "vertex.h"

#include <algorithm>
#include <optional>

const uint32_t WIDTH = 800, HEIGHT = 600;

std::optional<PhysicalDevice> findDevice(Instance &instance, Surface &surface) {
  auto physicalDevices = PhysicalDevice::all(instance);
  std::println("Found {} physical devices", physicalDevices.size());

  std::vector<char const *> requiredExtensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  std::vector<uint32_t> ratings(physicalDevices.size(), 1);
  for (size_t i = 0; i < physicalDevices.size(); i++) {
    auto &device = physicalDevices[i];

    auto unsupportedExtensions =
        device.findUnsupportedExtensions(requiredExtensions);

    if (unsupportedExtensions.size() > 0) {
      ratings[i] = 0;
      continue;
    }

    // Need to do this after extensions, since this wil fail if the m_device
    // doesn't support swap chains
    auto surfaceAttrs = SurfaceAttributes(device, surface);

    if (surfaceAttrs.formats.size() == 0 ||
        surfaceAttrs.presentModes.size() == 0) {
      ratings[i] = 0;
      continue;
    }

    if (device.isDiscrete()) {
      ratings[i] += 1000;
    }
  }

  int maxIndex = static_cast<int>(
      std::max_element(ratings.begin(), ratings.end()) - ratings.begin());

  if (ratings[maxIndex] == 0)
    return std::nullopt;

  PhysicalDevice &physicalDevice = physicalDevices[maxIndex];

  auto swapChainSupport = SurfaceAttributes(physicalDevice, surface);

  std::println("Found {} device: {}",
               physicalDevice.isDiscrete() ? "discrete" : "integrated",
               physicalDevice.getProperties().deviceName);

  return physicalDevice;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR
pickPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &mode : availablePresentModes) {
    if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return mode;
    }
  }
  return VK_PRESENT_MODE_FIFO_KHR;
}

std::optional<RenderPass> createRenderPass(Device &device,
                                           Swapchain &swapChain) {
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

  auto rp = RenderPass::create(device, renderPassCreateInfo);

  return rp;
}

std::optional<Shader> makeShader(Device &device, const char *path,
                                 ShaderStage stage) {
  auto shader = Shader::fromFile(path, stage, device);
  if (!shader.has_value()) {
    std::cerr << "Failed to create shader." << std::endl;
    return std::nullopt;
  }

  return shader;
}

std::optional<GraphicsPipeline> makePipeline(Device &device,
                                             Swapchain &swapChain,
                                             PipelineLayout &layout,
                                             RenderPass &renderPass) {
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

  vk::info::PipelineMultisampleStateCreate multisampleState;

  vk::info::PipelineColorBlendStateCreate colorBlendState;

  vk::PipelineColorBlendAttachmentState colorBlendAttachment;

  colorBlendState.addAttachment(colorBlendAttachment);

  VkViewport viewport{.x = 0.,
                      .y = 0.,
                      .width = static_cast<float>(swapChain.getExtent().width),
                      .height =
                          static_cast<float>(swapChain.getExtent().height),
                      .minDepth = 0.,
                      .maxDepth = 1.0};

  VkRect2D scissor{.offset = {0, 0}, .extent = swapChain.getExtent()};

  vk::info::PiplineViewportStateCreate viewportState;

  viewportState.addViewport(viewport);
  viewportState.addScissor(scissor);

  auto vertexShader =
      makeShader(device, "shaders/build/basic.vert.spv", ShaderStage::Vertex);

  if (!vertexShader.has_value()) {
    std::cerr << "Failed to create vertex shader." << std::endl;
    return std::nullopt;
  }

  auto fragmentShader =
      makeShader(device, "shaders/build/basic.frag.spv", ShaderStage::Fragment);

  if (!fragmentShader.has_value()) {
    std::cerr << "Failed to create fragment shader." << std::endl;
    return std::nullopt;
  }

  vk::info::PipelineShaderStageCreate vertexShaderStage(*vertexShader);
  vk::info::PipelineShaderStageCreate fragmentShaderStage(*fragmentShader);

  VkPipelineShaderStageCreateInfo shaderStages[] = {vertexShaderStage,
                                                    fragmentShaderStage};

  vk::info::GraphicsPipelineCreate pipelineCreateInfo(
      *layout, *renderPass, 2, shaderStages, vertexInput, inputAssembly,
      viewportState, rasterizationState, multisampleState, colorBlendState,
      dynState, 0);

  auto pipeline = GraphicsPipeline::create(device, pipelineCreateInfo);

  if (!pipeline.has_value()) {
    std::cerr << "Failed to create graphics pipeline." << std::endl;
    return std::nullopt;
  }

  return pipeline;
}

std::optional<App::VBufferParts> createVertexBuffers(Device &device,
                                                     Queue &transferQueue) {
  using VertexT = Vertex;
  using IndexT = uint16_t;
  const int VERTEX_COUNT = 4;
  const int INDEX_COUNT = 6;

  VkDeviceSize bufSize = VERTEX_COUNT * sizeof(VertexT);
  LOG("Buffer size: {}", bufSize);

  vk::info::VertexBufferCreate vBufInfo(bufSize,
                                        VK_BUFFER_USAGE_TRANSFER_DST_BIT);

  auto vBuf_opt = device.createVertexBuffer(vBufInfo);
  if (!vBuf_opt.has_value()) {
    LOG_ERR("Failed to create vertex buffer");
    return std::nullopt;
  }
  auto &vBuf = vBuf_opt.value();

  VkDeviceSize indexBufSize = INDEX_COUNT * sizeof(IndexT);
  LOG("Index buffer size: {}", indexBufSize);

  vk::info::IndexBufferCreate indexBufInfo(indexBufSize,
                                           VK_BUFFER_USAGE_TRANSFER_DST_BIT);
  auto indexBuf_opt = device.createIndexBuffer(indexBufInfo);
  if (!indexBuf_opt.has_value()) {
    LOG_ERR("Failed to create index buffer");
    return std::nullopt;
  }
  auto &indexBuf = indexBuf_opt.value();

  std::array<Buffer *, 2> buffers = {&vBuf, &indexBuf};
  std::span<Buffer *> buffersSpan(buffers);

  auto memory_opt =
      device.allocateMemory(buffersSpan, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!memory_opt.has_value()) {
    LOG_ERR("Failed to allocate vertex buffer memory");
    return std::nullopt;
  }
  auto &memory = memory_opt.value();

  if (vBuf.bind(memory) != VK_SUCCESS) {
    LOG_ERR("Failed to bind vertex buffer memory");
    return std::nullopt;
  }

  if (indexBuf.bind(memory, bufSize) != VK_SUCCESS) {
    LOG_ERR("Failed to bind index buffer memory");
    return std::nullopt;
  }

  vk::info::CommandPoolCreate cmdPoolInfo(transferQueue.getFamilyIndex(), false,
                                          true);
  auto cmdPool_opt = device.createCommandPool(cmdPoolInfo);
  if (!cmdPool_opt.has_value()) {
    LOG_ERR("Failed to create command pool");
    return std::nullopt;
  }
  auto &cmdPool = cmdPool_opt.value();

  auto cmdBuf_opt = cmdPool.allocBuffer();
  if (!cmdBuf_opt.has_value()) {
    LOG_ERR("Failed to allocate command buffer");
    return std::nullopt;
  }
  auto &cmdBuf = cmdBuf_opt.value();

  vk::info::CommandBufferBegin cmdBufBegin{};
  cmdBufBegin.oneTime();

  std::vector<CommandBuffer::Encoder::TemporaryStaging> writes{};
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
        LOG_ERR("Failed to write buffer with staging");
        return std::nullopt;
      }
      writes.push_back(std::move(write.value()));
    }
    {
      std::array<IndexT, INDEX_COUNT> indices = {{0, 1, 2, 0, 2, 3}};
      std::span<IndexT> indicesSpan(indices);
      auto write = encoder.writeBufferWithStaging(indicesSpan, indexBuf);
      if (!write.has_value()) {
        LOG_ERR("Failed to write index buffer with staging");
        return std::nullopt;
      }
      writes.push_back(std::move(write.value()));
    }
  }

  auto fence_opt = device.createFence();
  if (!fence_opt.has_value()) {
    LOG_ERR("Failed to create fence");
    return std::nullopt;
  }
  auto &fence = fence_opt.value();

  transferQueue.submit(cmdBuf, fence);
  fence.wait();

  App::VBufferParts parts{
      .vertexBuffer = std::move(vBuf),
      .indexBuffer = std::move(indexBuf),
      .memory = std::move(memory),
  };

  return parts;
}

std::optional<App> App::create() {
  glfwInit();

  auto window_opt = Window::create("Vulkan App", WIDTH, HEIGHT);
  if (!window_opt.has_value()) {
    std::cerr << "Failed to create window." << std::endl;
    return std::nullopt;
  }
  auto &window = window_opt.value();

  vk::info::Application appInfo;
  vk::info::InstanceCreate createInfo(appInfo);
  auto instance_opt = Instance::create(createInfo);
  if (!instance_opt.has_value()) {
    std::cerr << "Failed to create Vulkan instance." << std::endl;
    return std::nullopt;
  }
  auto &instance = instance_opt.value();

  auto surface_opt = instance.createSurface(window);
  if (!surface_opt.has_value()) {
    std::cerr << "Failed to create surface." << std::endl;
    return std::nullopt;
  }
  auto &surface = surface_opt.value();

  auto phys_device_opt = findDevice(instance, surface);
  if (!phys_device_opt.has_value()) {
    std::cerr << "Failed to find a suitable physical device." << std::endl;
    return std::nullopt;
  }
  auto &physicalDevice = phys_device_opt.value();

  auto swapchainSupport = SurfaceAttributes(physicalDevice, surface);
  auto format = chooseSwapSurfaceFormat(swapchainSupport.formats);
  auto presentMode = pickPresentMode(swapchainSupport.presentModes);
  auto extent = window.getExtent(swapchainSupport.capabilities);

  auto queueFamilies = physicalDevice.getQueues();
  std::println("Found {} queue families", queueFamilies.size());

  std::optional<uint32_t> graphicsQueueFamilyIndex;
  std::optional<uint32_t> presentQueueFamilyIndex;

  for (size_t i = 0; i < queueFamilies.size(); i++) {
    bool graphics = queueFamilies[i].hasGraphics();
    bool present = queueFamilies[i].canPresentTo(surface);

    if (graphics && present) {
      graphicsQueueFamilyIndex = static_cast<uint32_t>(i);
      presentQueueFamilyIndex = static_cast<uint32_t>(i);
      break;
    } else if (graphics) {
      graphicsQueueFamilyIndex = static_cast<uint32_t>(i);
    } else if (present) {
      presentQueueFamilyIndex = static_cast<uint32_t>(i);
    }
  }

  if (!graphicsQueueFamilyIndex.has_value()) {
    std::cerr << "Failed to find a graphics queue family." << std::endl;
    return std::nullopt;
  }

  if (!presentQueueFamilyIndex.has_value()) {
    std::cerr << "Failed to find a present queue family." << std::endl;
    return std::nullopt;
  }

  vk::info::DeviceCreate deviceCreateInfo(physicalDevice.getFeatures());
  deviceCreateInfo.addQueue(*graphicsQueueFamilyIndex)
      .addQueue(*presentQueueFamilyIndex)
      .enableExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  std::println("Creating device with {} queues",
               deviceCreateInfo.queueCreateInfoCount);

  auto device_opt = Device::create(physicalDevice, deviceCreateInfo);

  if (!device_opt.has_value()) {
    std::cerr << "Failed to create logical device." << std::endl;
    return std::nullopt;
  }
  auto &device = device_opt.value();

  auto graphicsQueue = device.getQueue(graphicsQueueFamilyIndex.value(), 0);
  if (!graphicsQueue.has_value()) {
    std::cerr << "Failed to create graphics queue." << std::endl;
    return std::nullopt;
  }

  std::optional<PresentQueue> presentQueue =
      device.getQueue(presentQueueFamilyIndex.value(), 0);
  if (!presentQueue.has_value()) {
    std::cerr << "Failed to create present queue." << std::endl;
    return std::nullopt;
  }

  vk::info::SwapchainCreate swapChainCreateInfo(swapchainSupport, surface);
  swapChainCreateInfo.setImageFormat(format.format)
      .setImageColorSpace(format.colorSpace)
      .setImageExtent(extent)
      .setPresentMode(presentMode);

  if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
    std::vector<uint32_t> queueFamilyIndices = {
        static_cast<uint32_t>(graphicsQueueFamilyIndex.value()),
        static_cast<uint32_t>(presentQueueFamilyIndex.value())};
    swapChainCreateInfo.setImageSharingMode(VK_SHARING_MODE_CONCURRENT)
        .setQueueFamilyIndices(queueFamilyIndices);
  } else {
    swapChainCreateInfo.setImageSharingMode(VK_SHARING_MODE_EXCLUSIVE);
  }

  auto swapchain_opt = device.createSwapchain(swapChainCreateInfo);
  if (!swapchain_opt.has_value()) {
    std::cerr << "Failed to create swap chain." << std::endl;
    return std::nullopt;
  }
  auto &swapchain = swapchain_opt.value();

  vk::info::PipelineLayoutCreate pipelineLayoutCreateInfo;
  auto layout_opt = PipelineLayout::create(device, pipelineLayoutCreateInfo);
  if (!layout_opt.has_value()) {
    std::cerr << "Failed to create pipeline layout." << std::endl;
    return std::nullopt;
  }
  auto &layout = layout_opt.value();

  auto renderPass_opt = createRenderPass(device, swapchain);
  if (!renderPass_opt.has_value()) {
    std::cerr << "Failed to create render pass." << std::endl;
    return std::nullopt;
  }
  auto &renderPass = renderPass_opt.value();

  auto framebuffers_opt = swapchain.createFramebuffers(renderPass);
  if (!framebuffers_opt.has_value()) {
    std::cerr << "Failed to create framebuffers." << std::endl;
    return std::nullopt;
  }
  auto &framebuffers = framebuffers_opt.value();

  auto pipeline_opt = makePipeline(device, swapchain, layout, renderPass);
  if (!pipeline_opt.has_value()) {
    std::cerr << "Failed to create pipeline." << std::endl;
    return std::nullopt;
  }
  auto &pipeline = pipeline_opt.value();

  vk::info::CommandPoolCreate commandPoolCreateInfo(
      graphicsQueueFamilyIndex.value(), true);

  auto commandPool_opt = CommandPool::create(device, commandPoolCreateInfo);
  if (!commandPool_opt.has_value()) {
    std::cerr << "Failed to create command pool." << std::endl;
    return std::nullopt;
  }
  auto &commandPool = commandPool_opt.value();

  auto commandBuffer_opt = commandPool.allocBuffers(MAX_FRAMES_IN_FLIGHT);
  if (!commandBuffer_opt.has_value() ||
      commandBuffer_opt.value().size() != MAX_FRAMES_IN_FLIGHT) {
    std::cerr << "Failed to allocate command buffer." << std::endl;
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
    std::cerr << "Failed to create synchronization objects." << std::endl;
    return std::nullopt;
  }

  std::array<Frame, MAX_FRAMES_IN_FLIGHT> frames{
      {{.commandBuffer = commandBuffers[0],
        .imageAvailable = std::move(imageSem1.value()),
        .renderFinished = std::move(renderSem1.value()),
        .inFlight = std::move(inFlightFence1.value())},
       {.commandBuffer = commandBuffers[1],
        .imageAvailable = std::move(imageSem2.value()),
        .renderFinished = std::move(renderSem2.value()),
        .inFlight = std::move(inFlightFence2.value())}}};

  auto vBuf_opt = createVertexBuffers(device, graphicsQueue.value());
  if (!vBuf_opt.has_value()) {
    std::cerr << "Failed to create vertex buffer." << std::endl;
    return std::nullopt;
  }

  auto &vBuf = vBuf_opt.value();

  App app(window, instance, surface, device, graphicsQueue.value(),
          presentQueue.value(), swapchain, layout, renderPass, framebuffers,
          pipeline, commandPool, frames, vBuf);

  return std::move(app);
}
