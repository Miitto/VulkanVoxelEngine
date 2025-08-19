#include "app/app.hpp"
#include <algorithm>
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "engine/vulkan/queueFinder.hpp"
#include "logger.hpp"

#include <engine/core.hpp>
#include <engine/debug.hpp>
#include <engine/util/macros.hpp>
#include <engine/vulkan/extensions/pipeline.hpp>
#include <engine/vulkan/extensions/shader.hpp>
#include <engine/vulkan/physicalDeviceSelector.hpp>

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

auto pickPhysicalDevice(const vk::raii::Instance &instance) noexcept
    -> std::expected<vk::raii::PhysicalDevice, std::string> {
  EG_MAKE(physicalDeviceSelector,
          engine::vulkan::PhysicalDeviceSelector::create(instance),
          "Failed to create physical device selector");

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
                         const vk::raii::SurfaceKHR &surface) noexcept
    -> std::expected<std::tuple<vk::raii::Device, App::Queues>, std::string> {
  Logger::trace("Creating Logical Device");

  engine::vulkan::QueueFinder finder(physicalDevice);

  std::optional<uint32_t> graphicsQueueFamilyIndex_opt = std::nullopt;
  std::optional<uint32_t> presentQueueFamilyIndex_opt = std::nullopt;

  using engine::vulkan::QueueFinder;

  auto combinedFinder = finder.findCombined(
      {QueueFinder::QueueType{.type = QueueFinder::QueueTypeFlags::Graphics},
       {.type = QueueFinder::QueueTypeFlags::Present,
        .params = QueueFinder::QueueTypeParams{
            .presentQueue = {.device = physicalDevice, .surface = surface}}}});

  if (combinedFinder.hasQueue()) {
    auto &queue = combinedFinder.first();
    graphicsQueueFamilyIndex_opt = queue.index;
    presentQueueFamilyIndex_opt = queue.index;
  } else {
    auto graphicsFinder = finder.findType(
        QueueFinder::QueueType{.type = QueueFinder::QueueTypeFlags::Graphics});
    if (graphicsFinder.hasQueue()) {
      graphicsQueueFamilyIndex_opt = graphicsFinder.first().index;
    } else {
      Logger::error("No suitable graphics queue family found");
      return std::unexpected("No suitable graphics queue family found");
    }

    auto presentFinder = finder.findType(QueueFinder::QueueType{
        .type = QueueFinder::QueueTypeFlags::Present,
        .params = QueueFinder::QueueTypeParams{
            .presentQueue = {.device = physicalDevice, .surface = surface}}});

    if (presentFinder.hasQueue()) {
      presentQueueFamilyIndex_opt = presentFinder.first().index;
    } else {
      Logger::error("No suitable present queue family found");
      return std::unexpected("No suitable present queue family found");
    }
  }

  auto graphicsQueueFamilyIndex = graphicsQueueFamilyIndex_opt.value();

  auto presentQueueFamilyIndex = presentQueueFamilyIndex_opt.value();

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

  VK_MAKE(device, physicalDevice.createDevice(deviceCreateInfo),
          "Failed to create logical device");

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

auto createSwapchain(
    const vk::raii::PhysicalDevice &physicalDevice,
    const vk::raii::Device &device, GLFWwindow *window,
    const vk::raii::SurfaceKHR &surface, const App::Queues &queues,
    std::optional<vk::raii::SwapchainKHR *> oldSwapchain) noexcept
    -> std::expected<
        std::tuple<engine::vulkan::SwapchainConfig, engine::vulkan::Swapchain>,
        std::string> {
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

  auto format = engine::vulkan::chooseSwapSurfaceFormat(
      physicalDevice.getSurfaceFormatsKHR(surface));
  auto presentMode = engine::vulkan::chooseSwapPresentMode(
      physicalDevice.getSurfacePresentModesKHR(surface));
  auto extent =
      engine::vulkan::chooseSwapExtent(window, surfaceCapabilities, true);
  auto minImageCount =
      engine::vulkan::minImageCount(surfaceCapabilities, MAX_FRAMES_IN_FLIGHT);
  auto desiredImageCount =
      engine::vulkan::desiredImageCount(surfaceCapabilities);

  engine::vulkan::SwapchainConfig swapchainConfig{
      .format = format,
      .presentMode = presentMode,
      .extent = extent,
      .minImageCount = minImageCount,
      .imageCount = desiredImageCount};

  EG_MAKE(swapchain,
          engine::vulkan::Swapchain::create(
              device, swapchainConfig, physicalDevice, surface,
              {.graphicsQueueIndex = queues.graphicsQueue.index,
               .presentQueueIndex = queues.presentQueue.index},
              oldSwapchain),
          "Failed to create swapchain");

  return std::make_tuple(swapchainConfig, std::move(swapchain));
}

auto createSyncObjects(const vk::raii::Device &device) noexcept
    -> std::expected<App::SyncObjects, std::string> {
  VK_MAKE(presentCompleteSemaphore,
          device.createSemaphore(vk::SemaphoreCreateInfo{}),
          "Failed to create present complete semaphore");

  VK_MAKE(renderCompleteSemaphore,
          device.createSemaphore(vk::SemaphoreCreateInfo{}),
          "Failed to create render complete semaphore");

  VK_MAKE(drawingFence,
          device.createFence(
              vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled}),
          "Failed to create drawing fence");

  App::SyncObjects syncObjects{
      .presentCompleteSemaphore = std::move(presentCompleteSemaphore),
      .renderCompleteSemaphore = std::move(renderCompleteSemaphore),
      .drawingFence = std::move(drawingFence)};

  return syncObjects;
}

auto App::create() noexcept -> std::expected<App, std::string> {
  EG_MAKE(core,
          engine::rendering::Core::create({.width = WINDOW_WIDTH,
                                           .height = WINDOW_HEIGHT,
                                           .title = WINDOW_TITLE},
                                          {
                                              .extraExtensions = {},
                                              .extraLayers = {},
                                          },
                                          enableValidationLayers),
          "Failed to create rendering core");

  auto &instance = core.getInstance();
  auto &window = core.getWindow();
  auto &surface = core.getSurface();

  EG_MAKE(physicalDevice, pickPhysicalDevice(instance),
          "Failed to pick physical device");

  EG_MAKE(deviceParts, createLogicalDevice(physicalDevice, surface),
          "Failed to create logical device");
  auto &[device, queues] = deviceParts;

  EG_MAKE(swapchainParts,
          createSwapchain(physicalDevice, device, window.get(), surface, queues,
                          std::nullopt),
          "Failed to create swapchain");
  auto &[swapchainConfig, swapchain] = swapchainParts;

  VK_MAKE(commandPool,
          device.createCommandPool(vk::CommandPoolCreateInfo{
              .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
              .queueFamilyIndex = queues.graphicsQueue.index}),
          "Failed to create command pool");

  EG_MAKE(syncObjects1, createSyncObjects(device),
          "Failed to create sync objects for frame 1");

  EG_MAKE(syncObjects2, createSyncObjects(device),
          "Failed to create sync objects for frame 2");

  std::array<App::SyncObjects, MAX_FRAMES_IN_FLIGHT> syncObjects = {
      std::move(syncObjects1), std::move(syncObjects2)};

  Logger::trace("Creating App");
  App app(core, physicalDevice, device, queues, swapchainConfig, swapchain,
          commandPool, syncObjects);

  return app;
}

auto App::recreateSwapchain() noexcept -> std::expected<void, std::string> {
  Logger::trace("Recreating Swapchain");

  auto &window = core.getWindow();
  auto &surface = core.getSurface();

  EG_MAKE(newSwapchainParts,
          createSwapchain(physicalDevice, device, window.get(), surface, queues,
                          &*swapchain),
          "Failed to create new swapchain");
  auto &[newSwapchainConfig, newSwapchain] = newSwapchainParts;

  oldSwapchain = OldSwapchain{
      .swapchain = std::move(swapchain),
      .frameIndex = currentFrame,
  };

  swapchainConfig = newSwapchainConfig;
  swapchain = std::move(newSwapchain);

  Logger::trace("Swapchain recreated successfully");
  return {};
}
