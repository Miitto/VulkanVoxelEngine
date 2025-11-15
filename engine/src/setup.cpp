#include "engine/setup.hpp"
#include "logger.hpp"
#include <vkh/queueFinder.hpp>

namespace engine::setup {
std::expected<vk::raii::PhysicalDevice, std::string> selectPhysicalDevice(
    const vk::raii::Instance &instance,
    std::function<std::optional<std::string>(vkh::PhysicalDeviceSelector &)>
        sel) {
  EG_MAKE(physicalDeviceSelector, vkh::PhysicalDeviceSelector::create(instance),
          "Failed to create physical device selector");

  auto res = sel(physicalDeviceSelector);
  if (res.has_value()) {
    return std::unexpected(res.value());
  }

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

auto findCoreQueues(const vk::raii::PhysicalDevice &physicalDevice,
                    const vk::raii::SurfaceKHR &surface)
    -> std::expected<CoreQueueFamilyIndices, std::string> {
  using vkh::QueueFinder;

  QueueFinder finder(physicalDevice);

  auto combinedFinder = finder.findCombined(
      {{QueueFinder::QueueType{.type = QueueFinder::QueueTypeFlags::Graphics}},
       {QueueFinder::QueueType{.type = QueueFinder::QueueTypeFlags::Present,
                               .params = QueueFinder::QueueTypeParams{
                                   .presentQueue = {.device = physicalDevice,
                                                    .surface = surface}}}}});

  if (combinedFinder.hasQueue()) {
    auto &queue = combinedFinder.first();
    return CoreQueueFamilyIndices{.graphics = queue.index,
                                  .present = queue.index};
  }

  CoreQueueFamilyIndices ind{};

  auto graphicsFinder = finder.findType(
      QueueFinder::QueueType{.type = QueueFinder::QueueTypeFlags::Graphics});

  if (graphicsFinder.hasQueue()) {
    auto queue = graphicsFinder.first();
    ind.graphics = queue.index;
  } else {
    Logger::error("No graphics queue family found");
    return std::unexpected("No graphics queue family found");
  }

  auto presentFinder = finder.findType(QueueFinder::QueueType{
      .type = QueueFinder::QueueTypeFlags::Present,
      .params = QueueFinder::QueueTypeParams{
          .presentQueue = {.device = physicalDevice, .surface = surface}}});

  if (presentFinder.hasQueue()) {
    auto presentFamily = presentFinder.first();
    ind.present = presentFamily.index;
  } else {
    Logger::error("No present queue family found");
    return std::unexpected("No present queue family found");
  }

  return ind;
}

auto retrieveQueues(const vk::raii::Device &device,
                    const std::vector<uint32_t> &indices)
    -> std::expected<std::vector<vkh::Queue>, std::string> {
  std::vector<vkh::Queue> queues;
  queues.reserve(indices.size());

  for (const auto &index : indices) {
    bool found = false;
    for (const auto &q : queues) {
      if (q.index == index) {
        found = true;
        queues.push_back(q);
        break;
      }
    }
    if (found)
      continue;

    auto queue_res = device.getQueue(index, 0);
    if (!queue_res) {
      Logger::error("Failed to get queue at index {}: {}", index,
                    vk::to_string(queue_res.error()));
      return std::unexpected("Failed to get queue");
    }
    auto queue =
        std::make_shared<vk::raii::Queue>(std::move(queue_res.value()));
    queues.push_back(vkh::Queue{.index = index, .queue = queue});
  }
  return queues;
}

std::expected<std::tuple<vkh::SwapchainConfig, vkh::Swapchain>, std::string>
createSwapchain(const vk::raii::PhysicalDevice &physicalDevice,
                const vk::raii::Device &device, const Window &window,
                const vk::raii::SurfaceKHR &surface,
                const CoreQueueFamilyIndices &queues,
                std::optional<vk::raii::SwapchainKHR *> oldSwapchain) noexcept {
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

  auto format = vkh::chooseSwapSurfaceFormat(
      physicalDevice.getSurfaceFormatsKHR(surface));
  auto presentMode = vkh::chooseSwapPresentMode(
      physicalDevice.getSurfacePresentModesKHR(surface));

  auto framebufferSize = window.getFramebufferSize();

  auto extent = vkh::chooseSwapExtent(framebufferSize.x, framebufferSize.y,
                                      surfaceCapabilities, true);
  auto minImageCount =
      vkh::minImageCount(surfaceCapabilities, MAX_FRAMES_IN_FLIGHT);
  auto desiredImageCount = vkh::desiredImageCount(surfaceCapabilities);

  vkh::SwapchainConfig swapchainConfig{.format = format,
                                       .presentMode = presentMode,
                                       .extent = extent,
                                       .minImageCount = minImageCount,
                                       .imageCount = desiredImageCount};

  EG_MAKE(swapchain,
          vkh::Swapchain::create(device, swapchainConfig, physicalDevice,
                                 surface,
                                 {.graphicsQueueIndex = queues.graphics,
                                  .presentQueueIndex = queues.present},
                                 oldSwapchain),
          "Failed to create swapchain");

  return std::make_tuple(swapchainConfig, std::move(swapchain));
}

auto createSyncObjects(const vk::raii::Device &device) noexcept
    -> std::expected<SyncObjects, std::string> {
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

  SyncObjects syncObjects{
      .presentCompleteSemaphore = std::move(presentCompleteSemaphore),
      .renderCompleteSemaphore = std::move(renderCompleteSemaphore),
      .drawingFence = std::move(drawingFence)};

  return syncObjects;
}
} // namespace engine::setup
