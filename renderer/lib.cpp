#include "lib.h"
#include "device/physicalDevice.h"
#include "pipeline/graphics.h"
#include "pipeline/layout.h"
#include "pipeline/renderPass.h"
#include "queue.h"
#include "shaders/shader.h"
#include "swapchain.h"
#include "vkStructs/all.h"
#include "vkStructs/presentInfo.h"
#include "vkStructs/renderPass.h"
#include "vkStructs/submitInfo.h"
#include <algorithm>
#include <iostream>
#include <print>
#include <vulkan/vulkan_core.h>

const uint32_t WIDTH = 800, HEIGHT = 600;

std::optional<Instance> makeInstance() {
  auto appInfo = ApplicationInfoBuilder().build();
  auto instance = InstanceCreateInfoBuilder(appInfo)
                      .enableGLFWExtensions()
                      .enableValidationLayers()
                      .createInstance();

  return instance;
}

struct DeviceReturn {
  PhysicalDevice device;
  SurfaceAttributes swapchainSupport;
};

std::optional<DeviceReturn> findDevice(Instance &instance, Surface &surface) {
  auto physicalDevices = PhysicalDevice::all(instance);
  std::println("Found {} physical devices", physicalDevices.size());

  std::vector<uint32_t> ratings(physicalDevices.size(), 0);
  for (size_t i = 0; i < physicalDevices.size(); i++) {
    auto &device = physicalDevices[i];
    ratings[i] = 1;

    std::vector<char const *> requiredExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    auto unsupportedExtensions =
        device.findUnsupportedExtensions(requiredExtensions);

    if (unsupportedExtensions.size() > 0) {
      std::cerr << "Device " << i << " does not support required extensions: ";
      for (auto extension : unsupportedExtensions) {
        std::cerr << extension << " ";
      }
      std::cerr << std::endl;
      ratings[i] = 0;
      continue;
    }

    // Need to do this after extensions, since this wil fail if the device
    // doesn't support swap chains
    auto surfaceAttrs = SurfaceAttributes(device, surface);

    if (surfaceAttrs.formats.size() == 0 ||
        surfaceAttrs.presentModes.size() == 0) {
      std::cerr << "Device " << i
                << " does not support swap chain formats or present modes."
                << std::endl;
      ratings[i] = 0;
      continue;
    }

    if (device.isDiscrete()) {
      ratings[i] += 1000;
    }
  }

  uint32_t maxRating = 0;
  int maxIndex = -1;
  for (size_t i = 0; i < ratings.size(); i++) {
    if (ratings[i] > maxRating) {
      maxRating = ratings[i];
      maxIndex = i;
    }
  }

  if (maxRating == 0) {
    std::cerr << "Failed to find a suitable physical device." << std::endl;
    return std::nullopt;
  }

  PhysicalDevice &physicalDevice = physicalDevices[maxIndex];

  auto swapChainSupport = SurfaceAttributes(physicalDevice, surface);

  if (physicalDevice.isDiscrete()) {
    std::println("Found discrete device: {}",
                 physicalDevice.getProperties().deviceName);
  } else {
    std::println("Found integrated device: {}",
                 physicalDevice.getProperties().deviceName);
  }

  DeviceReturn deviceReturn(std::move(physicalDevice), swapChainSupport);

  return std::move(deviceReturn);
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

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities,
                            Window &window) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    int width, height;
    glfwGetFramebufferSize(*window, &width, &height);

    VkExtent2D actualExtent = {.width = static_cast<uint32_t>(width),
                               .height = static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}

std::optional<PipelineLayout> makePipelineLayout(Device &device) {
  PipelineLayoutCreateInfoBuilder pipelineLayoutCreateInfo;

  auto pipelineLayout =
      PipelineLayout::create(device, pipelineLayoutCreateInfo.build());
  if (!pipelineLayout.has_value()) {
    std::cerr << "Failed to create pipeline layout." << std::endl;
    return std::nullopt;
  }

  return pipelineLayout;
}

std::optional<RenderPass> createRenderPass(Device &device,
                                           Swapchain &swapChain) {
  AttachmentDescriptionBuilder colorAttachment(swapChain.getFormat());

  colorAttachment.setColorDepth(VK_ATTACHMENT_LOAD_OP_CLEAR,
                                VK_ATTACHMENT_STORE_OP_STORE);

  colorAttachment.setFinalLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VkAttachmentReference colorAttachmentRef = {
      .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

  SubpassDescriptionBuilder subpassDescription;
  subpassDescription.color(colorAttachmentRef);

  RenderPassCreateInfoBuilder renderPassCreateInfo;
  renderPassCreateInfo.addAttachment(colorAttachment.build());
  renderPassCreateInfo.addSubpass(subpassDescription.build());

  VkSubpassDependency subpassDependency =
      SubpassDependencyBuilder(VK_SUBPASS_EXTERNAL, 0)
          .setSrcStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
          .setDstStageMask(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
          .setSrcAccessMask(0)
          .setDstAccessMask(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
          .build();

  renderPassCreateInfo.addDependency(subpassDependency);

  auto rp = RenderPass::create(device, renderPassCreateInfo.build());

  return rp;
}

std::optional<Shader> makeShader(Device &device, const char *path,
                                 EShaderStage stage) {
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
  PipelineDynamicStateCreateInfoBuilder dynState;
  dynState.addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
  dynState.addDynamicState(VK_DYNAMIC_STATE_SCISSOR);

  PipelineInputAssemblyStateCreateInfoBuilder inputAssembly;
  inputAssembly.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);

  PipelineVertexInputStateCreateInfoBuilder vertexInput;

  PipelineRasterizationStateCreateInfoBuilder rasterizationState;

  PipelineMultisampleStateCreateInfoBuilder multisampleState;

  PipelineColorBlendStateCreateInfoBuilder colorBlendState;

  PipelineColorBlendAttachmentStateBuilder colorBlendAttachment;

  colorBlendState.addAttachment(colorBlendAttachment.build());

  VkViewport viewport{.x = 0.,
                      .y = 0.,
                      .width = static_cast<float>(swapChain.getExtent().width),
                      .height =
                          static_cast<float>(swapChain.getExtent().height),
                      .minDepth = 0.,
                      .maxDepth = 1.0};

  VkRect2D scissor{.offset = {0, 0}, .extent = swapChain.getExtent()};

  PiplineViewportStateCreateInfoBuilder viewportState;

  viewportState.addViewport(viewport);
  viewportState.addScissor(scissor);

  auto vertexShader =
      makeShader(device, "shaders/build/basic.vert.spv", EShaderStage::VERTEX);

  if (!vertexShader.has_value()) {
    std::cerr << "Failed to create vertex shader." << std::endl;
    return std::nullopt;
  }

  auto fragmentShader =
      makeShader(device, "shaders/build/basic.frag.spv", EShaderStage::FRAG);

  if (!fragmentShader.has_value()) {
    std::cerr << "Failed to create fragment shader." << std::endl;
    return std::nullopt;
  }

  PipelineShaderStageCreateInfoBuilder vertexShaderStage(*vertexShader);
  PipelineShaderStageCreateInfoBuilder fragmentShaderStage(*fragmentShader);

  VkPipelineShaderStageCreateInfo shaderStages[] = {
      vertexShaderStage.build(), fragmentShaderStage.build()};

  GraphicsPipelineCreateInfoBuilder pipelineCreateInfo(
      *layout, *renderPass, 2, shaderStages, vertexInput.build(),
      inputAssembly.build(), viewportState.build(), rasterizationState.build(),
      multisampleState.build(), colorBlendState.build(), dynState.build(), 0);

  auto pipeline = GraphicsPipeline::create(device, pipelineCreateInfo.build());

  if (!pipeline.has_value()) {
    std::cerr << "Failed to create graphics pipeline." << std::endl;
    return std::nullopt;
  }

  return pipeline;
}

std::optional<App> App::create() {
  glfwInit();

  auto window_opt = Window::create("Vulkan App", WIDTH, HEIGHT);
  if (!window_opt.has_value()) {
    std::cerr << "Failed to create window." << std::endl;
    return std::nullopt;
  }
  auto &window = window_opt.value();

  auto instance_opt = makeInstance();
  if (!instance_opt.has_value()) {
    std::cerr << "Failed to create Vulkan instance." << std::endl;
    return std::nullopt;
  }
  auto &instance = instance_opt.value();

  auto surface_opt = Surface::create(instance, window);
  if (!surface_opt.has_value()) {
    std::cerr << "Failed to create surface." << std::endl;
    return std::nullopt;
  }
  auto &surface = surface_opt.value();

  auto deviceReturn = findDevice(instance, surface);
  if (!deviceReturn.has_value()) {
    std::cerr << "Failed to find a suitable physical device." << std::endl;
    return std::nullopt;
  }

  auto &physicalDevice = deviceReturn->device;
  auto swapchainSupport = deviceReturn->swapchainSupport;
  auto format = chooseSwapSurfaceFormat(swapchainSupport.formats);
  auto presentMode = pickPresentMode(swapchainSupport.presentModes);
  auto extent = chooseSwapExtent(swapchainSupport.capabilities, window);

  auto queueFamilies = physicalDevice.getQueues();
  std::println("Found {} queue families", queueFamilies.size());

  std::optional<int> graphicsQueueFamilyIndex;
  std::optional<int> presentQueueFamilyIndex;

  for (size_t i = 0; i < queueFamilies.size(); i++) {
    bool graphics = queueFamilies[i].hasGraphics();
    bool present = queueFamilies[i].canPresentTo(surface);

    if (graphics && present) {
      graphicsQueueFamilyIndex = i;
      presentQueueFamilyIndex = i;
      break;
    } else if (graphics) {
      graphicsQueueFamilyIndex = i;
    } else if (present) {
      presentQueueFamilyIndex = i;
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

  DeviceQueueCreateInfoBuilder graphicsQueueCreateInfo(
      graphicsQueueFamilyIndex.value());
  DeviceQueueCreateInfoBuilder presentQueueCreateInfo(
      presentQueueFamilyIndex.value());

  DeviceCreateInfoBuilder deviceCreateInfo;
  deviceCreateInfo.setPhysicalDeviceFeatures(physicalDevice.getFeatures());
  deviceCreateInfo.addQueueCreateInfo(
      graphicsQueueCreateInfo.setQueueCount(1).build());
  deviceCreateInfo.addQueueCreateInfo(
      presentQueueCreateInfo.setQueueCount(1).build());

  deviceCreateInfo.enableExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkDeviceCreateInfo deviceCreateInfoStruct = deviceCreateInfo.build();

  auto device_opt = Device::create(physicalDevice, deviceCreateInfoStruct);

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

  SwapChainCreateInfoBuilder swapChainCreateInfo(swapchainSupport, surface);
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

  auto swapchain_opt = Swapchain::create(device, swapChainCreateInfo.build());
  if (!swapchain_opt.has_value()) {
    std::cerr << "Failed to create swap chain." << std::endl;
    return std::nullopt;
  }
  auto &swapchain = swapchain_opt.value();

  auto layout_opt = makePipelineLayout(device);
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

  CommandPoolCreateInfoBuilder commandPoolCreateInfo(
      graphicsQueueFamilyIndex.value());

  commandPoolCreateInfo.resetable();

  auto commandPool_opt =
      CommandPool::create(device, commandPoolCreateInfo.build());
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

  auto imageSem1 = Semaphore::create(device);
  auto renderSem1 = Semaphore::create(device);
  auto inFlightFence1 = Fence::create(device, true);
  auto imageSem2 = Semaphore::create(device);
  auto renderSem2 = Semaphore::create(device);
  auto inFlightFence2 = Fence::create(device, true);

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

  App app(window, instance, surface, device, graphicsQueue.value(),
          presentQueue.value(), swapchain, layout, renderPass, framebuffers,
          pipeline, commandPool, frames);

  return std::move(app);
}

void App::run() {
  std::println("Running app...");
  while (!window.shouldClose()) {
    glfwPollEvents();

    update();
    render();
  }
  std::println("App closed.");
}

void App::update() {}

void App::render() {
  frames[currentFrame].inFlight.wait();
  auto [swpachainState, imageIndex] =
      swapchain.getNextImage(*frames[currentFrame].imageAvailable);

  switch (swpachainState) {
  case VK_SUBOPTIMAL_KHR:
  case VK_ERROR_OUT_OF_DATE_KHR:
    std::println("TODO: Recreate swapchain");
    return;
  case VK_SUCCESS:
    break;
  default: {
    std::cerr << "Failed to acquire swapchain image." << std::endl;
    return;
  }
  }

  frames[currentFrame].inFlight.reset();

  frames[currentFrame].commandBuffer.reset();
  recordCommandBuffer(frames[currentFrame].commandBuffer, imageIndex);

  SubmitInfoBuilder submitInfoBuilder;
  submitInfoBuilder
      .addWaitSemaphore(*frames[currentFrame].imageAvailable,
                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT)
      .addSignalSemaphore(*frames[currentFrame].renderFinished)
      .addCommandBuffer(*frames[currentFrame].commandBuffer);

  auto submitInfo = submitInfoBuilder.build();

  if (graphicsQueue.submit(submitInfo, *frames[currentFrame].inFlight) !=
      VK_SUCCESS) {
    std::cerr << "Failed to submit draw command buffer." << std::endl;
    return;
  }

  PresentInfoBuilder presentInfoBuilder =
      PresentInfoBuilder()
          .addWaitSemaphore(*frames[currentFrame].renderFinished)
          .addSwapchain(*swapchain)
          .setImageIndex(imageIndex);

  auto presentInfo = presentInfoBuilder.build();

  presentQueue.present(presentInfo);

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::recordCommandBuffer(CommandBuffer &commandBuffer,
                              uint32_t imageIndex) {
  auto encoder = commandBuffer.begin();

  RenderPassBeginInfoBuilder renderPassInfo(
      *renderPass, *framebuffers[imageIndex],
      VkRect2D{.offset = {0, 0}, .extent = swapchain.getExtent()});

  renderPassInfo.addClearValue({.color = {{0.0f, 0.0f, 0.0f, 1.0f}}});

  auto pass = encoder.beginRenderPass(renderPassInfo.build());

  pass.bindPipeline(pipeline);

  VkViewport viewport = {
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(swapchain.getExtent().width),
      .height = static_cast<float>(swapchain.getExtent().height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};

  VkRect2D scissor = {.offset = {0, 0}, .extent = swapchain.getExtent()};

  pass.setViewport(viewport);
  pass.setScissor(scissor);

  pass.draw(3);

  pass.end();

  encoder.end();
}

App::~App() {
  if (moveGuard.isMoved()) {
    return;
  }

  device.waitIdle();

  glfwTerminate();
}
