#include "app/app.hpp"
#include <algorithm>
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "logger.hpp"

#include <engine/core.hpp>
#include <engine/debug.hpp>
#include <engine/vulkan/physicalDeviceSelector.hpp>
#include <engine/vulkan/pipeline.hpp>
#include <engine/vulkan/shaders.hpp>
#include <engine/vulkan/swapchainConfig.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char *WINDOW_TITLE = "Vulkan App";

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::array<const char *, 3> requiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName};

auto pickPhysicalDevice(const vk::raii::Instance &instance)
    -> std::expected<vk::raii::PhysicalDevice, std::string> {
  auto physicalDeviceSelector_res =
      engine::vulkan::PhysicalDeviceSelector::create(instance);

  if (!physicalDeviceSelector_res) {
    Logger::error("Failed to create Physical Device Selector: {}",
                  physicalDeviceSelector_res.error());
    return std::unexpected(physicalDeviceSelector_res.error());
  }

  auto &physicalDeviceSelector = physicalDeviceSelector_res.value();

  physicalDeviceSelector.requireExtensions(requiredDeviceExtensions);
  physicalDeviceSelector.requireVersion(1, 4, 0);
  physicalDeviceSelector.requireQueueFamily(vk::QueueFlagBits::eGraphics);

  physicalDeviceSelector.scoreDevices(
      [](const engine::vulkan::PhysicalDeviceSelector::DeviceSpecs &spec) {
        uint32_t score = 0;
        if (spec.properties.deviceType ==
            vk::PhysicalDeviceType::eDiscreteGpu) {
          score += 1000;
        }

        return score;
      });

  physicalDeviceSelector.sortDevices();

  auto physicalDevices = physicalDeviceSelector.select();

  if (physicalDevices.empty()) {
    Logger::error("No suitable physical devices found");
    return std::unexpected("No suitable physical devices found");
  }

  Logger::info("Found {} suitable physical devices", physicalDevices.size());
  const auto &physicalDevice = physicalDevices.front();

  Logger::info("Using physical device: {}",
               physicalDevice.getProperties().deviceName.data());

  return physicalDevice;
}

auto createLogicalDevice(const vk::raii::PhysicalDevice &physicalDevice,
                         const vk::raii::SurfaceKHR &surface)
    -> std::expected<std::tuple<vk::raii::Device, App::Queues>, std::string> {
  Logger::trace("Creating Logical Device");

  auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

  auto enumerated = std::ranges::views::enumerate(queueFamilyProperties);

  auto combinedFinder = [&physicalDevice, &surface](auto p) {
    auto &[index, props] = p;
    return bool(props.queueFlags & vk::QueueFlagBits::eGraphics) &&
           physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(index),
                                               surface);
  };

  auto graphicsQueueFamilyIndex = 0u;
  auto presentQueueFamilyIndex = 0u;

  auto combinedQueueFamilyIndex = static_cast<uint32_t>(std::distance(
      enumerated.begin(), std::ranges::find_if(enumerated, combinedFinder)));

  if (combinedQueueFamilyIndex == queueFamilyProperties.size()) {
    auto graphicsFinder = [](auto const &props) {
      return bool(props.queueFlags & vk::QueueFlagBits::eGraphics);
    };

    auto presentFinder = [&physicalDevice, &surface](auto const &p) {
      auto &[index, props] = p;
      return physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(index),
                                                 surface);
    };

    auto graphicsQueueFamilyIt =
        std::ranges::find_if(queueFamilyProperties, graphicsFinder);
    auto presentQueueFamilyIt = std::ranges::find_if(enumerated, presentFinder);

    if (graphicsQueueFamilyIt == queueFamilyProperties.end() ||
        presentQueueFamilyIt == enumerated.end()) {
      Logger::error("No suitable graphics or present queue family found");
      return std::unexpected(
          "No suitable graphics or present queue family found");
    }

    graphicsQueueFamilyIndex = static_cast<uint32_t>(
        std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyIt));
    presentQueueFamilyIndex = static_cast<uint32_t>(
        std::distance(enumerated.begin(), presentQueueFamilyIt));
  } else {
    graphicsQueueFamilyIndex = combinedQueueFamilyIndex;
    presentQueueFamilyIndex = combinedQueueFamilyIndex;
  }

  Logger::trace("Found Graphics Queue Family at index {}",
                graphicsQueueFamilyIndex);

  std::array<float, 1> queuePriority = {1.0f};

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo = {
      vk::DeviceQueueCreateInfo{.queueFamilyIndex = graphicsQueueFamilyIndex,
                                .queueCount = 1,
                                .pQueuePriorities = queuePriority.data()}};

  if (presentQueueFamilyIndex != graphicsQueueFamilyIndex) {
    Logger::trace("Found Present Queue Family at index {}",
                  presentQueueFamilyIndex);
    queueCreateInfo.push_back(
        vk::DeviceQueueCreateInfo{.queueFamilyIndex = presentQueueFamilyIndex,
                                  .queueCount = 1,
                                  .pQueuePriorities = queuePriority.data()});
  }

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan11Features,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featursChain = {{},
                      {.shaderDrawParameters = true},
                      {.synchronization2 = true, .dynamicRendering = true},
                      {.extendedDynamicState = true}};

  auto deviceCreateInfo = vk::DeviceCreateInfo{
      .pNext = &featursChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size()),
      .pQueueCreateInfos = queueCreateInfo.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(requiredDeviceExtensions.size()),
      .ppEnabledExtensionNames = requiredDeviceExtensions.data()};

  Logger::trace("Creating logical device");

  std::expected<vk::raii::Device, vk::Result> device_res =
      physicalDevice.createDevice(deviceCreateInfo);

  Logger::trace("Logical device created");

  if (!device_res) {
    Logger::error("Failed to create logical device: {}",
                  vk::to_string(device_res.error()));
    return std::unexpected("Failed to create logical device");
  }

  auto &device = device_res.value();

  auto graphicsQueue_res = device.getQueue(graphicsQueueFamilyIndex, 0);
  if (!graphicsQueue_res) {
    Logger::error("Failed to get graphics queue: {}",
                  vk::to_string(graphicsQueue_res.error()));
    return std::unexpected("Failed to get graphics queue");
  }
  auto graphicsQueue =
      std::make_shared<vk::raii::Queue>(std::move(graphicsQueue_res.value()));

  std::shared_ptr<vk::raii::Queue> presentQueue = nullptr;

  if (presentQueueFamilyIndex != graphicsQueueFamilyIndex) {
    auto presentQueue_res = device.getQueue(presentQueueFamilyIndex, 0);
    if (!presentQueue_res) {
      Logger::error("Failed to get present queue: {}",
                    vk::to_string(presentQueue_res.error()));
      return std::unexpected("Failed to get present queue");
    }
    presentQueue =
        std::make_shared<vk::raii::Queue>(std::move(presentQueue_res.value()));
  } else {
    presentQueue = graphicsQueue;
  }

  App::Queues queues{
      .graphicsQueue =
          App::Queue{.index = graphicsQueueFamilyIndex, .queue = graphicsQueue},
      .presentQueue =
          App::Queue{.index = presentQueueFamilyIndex, .queue = presentQueue}};

  return std::make_tuple(std::move(device), queues);
}

auto createSwapchain(const vk::raii::PhysicalDevice &physicalDevice,
                     const vk::raii::Device &device, GLFWwindow *window,
                     const vk::raii::SurfaceKHR &surface,
                     const App::Queues &queues,
                     std::optional<vk::raii::SwapchainKHR *> oldSwapchain)
    -> std::expected<std::tuple<App::SwapchainConfig, App::Swapchain>,
                     std::string> {
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

  auto format = engine::vulkan::chooseSwapSurfaceFormat(
      physicalDevice.getSurfaceFormatsKHR(surface));
  auto presentMode = engine::vulkan::chooseSwapPresentMode(
      physicalDevice.getSurfacePresentModesKHR(surface));
  auto extent = engine::vulkan::chooseSwapExtent(window, surfaceCapabilities);
  auto minImageCount =
      engine::vulkan::minImageCount(surfaceCapabilities, MAX_FRAMES_IN_FLIGHT);
  auto desiredImageCount =
      engine::vulkan::desiredImageCount(surfaceCapabilities);

  App::SwapchainConfig swapchainConfig{.format = format,
                                       .presentMode = presentMode,
                                       .extent = extent,
                                       .minImageCount = minImageCount,
                                       .imageCount = desiredImageCount};

  vk::SwapchainCreateInfoKHR swapchainCreateInfo{
      .surface = *surface,
      .minImageCount = swapchainConfig.minImageCount,
      .imageFormat = swapchainConfig.format.format,
      .imageColorSpace = swapchainConfig.format.colorSpace,
      .imageExtent = swapchainConfig.extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .preTransform = surfaceCapabilities.currentTransform,
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = swapchainConfig.presentMode,
      .clipped = VK_TRUE};

  std::array<uint32_t, 2> queueFamilyIndices = {queues.graphicsQueue.index,
                                                queues.presentQueue.index};

  if (queues.graphicsQueue.index != queues.presentQueue.index) {
    swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
    swapchainCreateInfo.queueFamilyIndexCount = 2;
    swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
  } else {
    swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
  }

  if (oldSwapchain) {
    swapchainCreateInfo.oldSwapchain = **oldSwapchain;
  } else {
    swapchainCreateInfo.oldSwapchain = nullptr;
  }

  auto swapchain_res = device.createSwapchainKHR(swapchainCreateInfo);
  if (!swapchain_res) {
    Logger::error("Failed to create swapchain: {}",
                  vk::to_string(swapchain_res.error()));
    return std::unexpected("Failed to create swapchain");
  }

  auto &swapchain = swapchain_res.value();

  auto images = swapchain.getImages();

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .viewType = vk::ImageViewType::e2D,
      .format = swapchainConfig.format.format,
      .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  std::vector<vk::raii::ImageView> imageViews;

  for (const auto &image : images) {
    imageViewCreateInfo.image = image;
    auto imageView_res = device.createImageView(imageViewCreateInfo);
    if (!imageView_res) {
      Logger::error("Failed to create image view: {}",
                    vk::to_string(imageView_res.error()));
      return std::unexpected("Failed to create image view");
    }
    imageViews.push_back(std::move(imageView_res.value()));
  }

  App::Swapchain swapchain_objects{.swapchain = std::move(swapchain),
                                   .images = std::move(images),
                                   .imageViews = std::move(imageViews)};

  return std::make_tuple(swapchainConfig, std::move(swapchain_objects));
}

auto createPipeline(const vk::raii::Device &device,
                    const App::SwapchainConfig &swapchainConfig,
                    const engine::PipelineShaderStages shaderStages)
    -> std::expected<vk::raii::Pipeline, std::string> {
  Logger::trace("Creating Graphics Pipeline");
  engine::DynamicStateInfo dynamicStateInfo(vk::DynamicState::eViewport,
                                            vk::DynamicState::eScissor);

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      .topology = vk::PrimitiveTopology::eTriangleStrip};

  vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1,
                                                    .scissorCount = 1};

  vk::PipelineRasterizationStateCreateInfo rasterizer{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
      .depthBiasEnable = vk::False,
      .depthBiasSlopeFactor = 1.0f,
      .lineWidth = 1.0f};

  vk::PipelineMultisampleStateCreateInfo multisampling{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False,
      .minSampleShading = 1.0f};

  vk::PipelineColorBlendAttachmentState colorBlendAttachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colorBlending{
      .logicOpEnable = vk::False,
      .attachmentCount = 1,
      .pAttachments = &colorBlendAttachment};

  vk::PipelineLayoutCreateInfo layoutInfo{.setLayoutCount = 0,
                                          .pushConstantRangeCount = 0};

  auto pipelineLayout_res = device.createPipelineLayout(layoutInfo);
  if (!pipelineLayout_res) {
    Logger::error("Failed to create pipeline layout: {}",
                  vk::to_string(pipelineLayout_res.error()));
    return std::unexpected("Failed to create pipeline layout");
  }

  auto &pipelineLayout = pipelineLayout_res.value();

  vk::PipelineRenderingCreateInfo renderingCreateInfo{
      .colorAttachmentCount = 1,
      .pColorAttachmentFormats = &swapchainConfig.format.format,
  };

  vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
      .pNext = &renderingCreateInfo,
      .stageCount = shaderStages.size(),
      .pStages = shaderStages.data(),
      .pVertexInputState = &vertexInputInfo,
      .pInputAssemblyState = &inputAssembly,
      .pViewportState = &viewportState,
      .pRasterizationState = &rasterizer,
      .pMultisampleState = &multisampling,
      .pColorBlendState = &colorBlending,
      .pDynamicState = dynamicStateInfo,
      .layout = pipelineLayout,
      .renderPass = nullptr,
  };

  Logger::trace("Creating Graphics Pipeline");
  auto pipeline_res =
      device.createGraphicsPipeline(nullptr, pipelineCreateInfo);

  if (!pipeline_res) {
    Logger::error("Failed to create graphics pipeline: {}",
                  vk::to_string(pipeline_res.error()));
    return std::unexpected("Failed to create graphics pipeline");
  }
  Logger::trace("Graphics Pipeline created");

  auto &pipeline = pipeline_res.value();

  return std::move(pipeline);
}

auto createSyncObjects(const vk::raii::Device &device)
    -> std::expected<App::SyncObjects, std::string> {
  auto presentCompleteSemaphore_res =
      device.createSemaphore(vk::SemaphoreCreateInfo{});
  if (!presentCompleteSemaphore_res) {
    Logger::error("Failed to create present complete semaphore: {}",
                  vk::to_string(presentCompleteSemaphore_res.error()));
    return std::unexpected("Failed to create present complete semaphore");
  }
  auto &presentCompleteSemaphore = presentCompleteSemaphore_res.value();

  auto renderCompleteSemaphore_res =
      device.createSemaphore(vk::SemaphoreCreateInfo{});
  if (!renderCompleteSemaphore_res) {
    Logger::error("Failed to create render complete semaphore: {}",
                  vk::to_string(renderCompleteSemaphore_res.error()));
    return std::unexpected("Failed to create render complete semaphore");
  }

  auto &renderCompleteSemaphore = renderCompleteSemaphore_res.value();

  auto drawingFence_res = device.createFence(
      vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
  if (!drawingFence_res) {
    Logger::error("Failed to create drawing fence: {}",
                  vk::to_string(drawingFence_res.error()));
    return std::unexpected("Failed to create drawing fence");
  }
  auto &drawingFence = drawingFence_res.value();

  App::SyncObjects syncObjects{
      .presentCompleteSemaphore = std::move(presentCompleteSemaphore),
      .renderCompleteSemaphore = std::move(renderCompleteSemaphore),
      .drawingFence = std::move(drawingFence)};

  return syncObjects;
}

auto App::create() -> std::expected<App, std::string> {
  auto core_res = engine::rendering::Core::create(
      {.width = WINDOW_WIDTH, .height = WINDOW_HEIGHT, .title = WINDOW_TITLE},
      {
          .extraExtensions = {},
          .extraLayers = {},
      },
      enableValidationLayers);

  if (!core_res) {
    Logger::critical("Failed to create rendering core: {}", core_res.error());
    return std::unexpected(core_res.error());
  }
  auto &core = core_res.value();

  auto &instance = core.getInstance();
  auto &window = core.getWindow();
  auto &surface = core.getSurface();

  auto physicalDevice_res = pickPhysicalDevice(instance);
  if (!physicalDevice_res) {
    return std::unexpected(physicalDevice_res.error());
  }
  auto &physicalDevice = physicalDevice_res.value();

  auto device_res = createLogicalDevice(physicalDevice, surface);

  if (!device_res) {
    return std::unexpected(device_res.error());
  }

  [[maybe_unused]] auto &[device, queues] = device_res.value();

  auto swapchain_res = createSwapchain(physicalDevice, device, window.get(),
                                       surface, queues, std::nullopt);

  if (!swapchain_res) {
    Logger::error("Failed to create swapchain: {}", swapchain_res.error());
    return std::unexpected(swapchain_res.error());
  }

  auto &[swapchainConfig, swapchain] = swapchain_res.value();

  auto shader_res = engine::createShaderModule(device, "basic.spv");

  if (!shader_res) {
    Logger::error("Failed to create shader module: {}", shader_res.error());
    return std::unexpected(shader_res.error());
  }

  auto &shaderModule = shader_res.value();

  [[maybe_unused]]
  auto pipelineShaderStages = engine::createPipelineShaderStages(shaderModule);

  auto pipeline_res =
      createPipeline(device, swapchainConfig, pipelineShaderStages);
  if (!pipeline_res) {
    Logger::error("Failed to create pipeline: {}", pipeline_res.error());
    return std::unexpected(pipeline_res.error());
  }

  auto &pipeline = pipeline_res.value();

  auto commandPool_res = device.createCommandPool(vk::CommandPoolCreateInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = queues.graphicsQueue.index});
  if (!commandPool_res) {
    Logger::error("Failed to create command pool: {}",
                  vk::to_string(commandPool_res.error()));
    return std::unexpected("Failed to create command pool");
  }

  auto &commandPool = commandPool_res.value();

  auto syncObjects_res1 = createSyncObjects(device);
  if (!syncObjects_res1) {
    Logger::error("Failed to create sync objects: {}",
                  syncObjects_res1.error());
    return std::unexpected(syncObjects_res1.error());
  }

  auto &syncObjects1 = syncObjects_res1.value();

  auto syncObjects_res2 = createSyncObjects(device);
  if (!syncObjects_res2) {
    Logger::error("Failed to create sync objects: {}",
                  syncObjects_res2.error());
    return std::unexpected(syncObjects_res2.error());
  }

  auto &syncObjects2 = syncObjects_res2.value();

  std::array<App::SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects = {
      std::move(syncObjects1), std::move(syncObjects2)};

  Logger::trace("Creating App");
  App app(core, physicalDevice, device, queues, swapchainConfig, swapchain,
          pipeline, commandPool, syncObjects);

  return app;
}

auto App::recreateSwapchain() -> std::expected<void, std::string> {
  Logger::trace("Recreating Swapchain");

  auto &window = core.getWindow();
  auto &surface = core.getSurface();

  auto newSwapchain_res =
      createSwapchain(physicalDevice, device, window.get(), surface, queues,
                      &swapchain.swapchain);
  if (!newSwapchain_res) {
    Logger::error("Failed to recreate swapchain: {}", newSwapchain_res.error());
    return std::unexpected(newSwapchain_res.error());
  }

  auto &[newSwapchainConfig, newSwapchain] = newSwapchain_res.value();

  oldSwapchain = OldSwapchain{
      .swapchain = std::move(swapchain.swapchain),
      .frameIndex = currentFrame,
  };

  swapchainConfig = newSwapchainConfig;
  swapchain = std::move(newSwapchain);

  Logger::trace("Swapchain recreated successfully");
  return {};
}
