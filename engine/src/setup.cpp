#include "engine/setup.hpp"
#include "engine/defines.hpp"
#include "logger.hpp"
#include <vkh/queueFinder.hpp>

#include <imgui/imgui.h>

#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

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

std::expected<vkh::Swapchain, std::string>
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

  return std::move(swapchain);
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

std::expected<vkh::AllocatedImage, std::string>
createRenderImage(const vk::raii::Device &device, const vma::Allocator &alloc,
                  const vkh::SwapchainConfig &swapchainConfig,
                  vk::Format format) {
  vk::ImageCreateInfo imageCreateInfo{
      .imageType = vk::ImageType::e2D,
      .format = format,
      .extent = vk::Extent3D{.width = swapchainConfig.extent.width,
                             .height = swapchainConfig.extent.height,
                             .depth = 1},
      .mipLevels = 1,
      .arrayLayers = 1,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eColorAttachment |
               vk::ImageUsageFlagBits::eTransferSrc |
               vk::ImageUsageFlagBits::eStorage,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  vma::AllocationCreateInfo allocInfo{
      .usage = vma::MemoryUsage::eGpuOnly,
      .requiredFlags =
          vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal)};

  auto imageRes = alloc.createImage(imageCreateInfo, allocInfo);
  if (imageRes.result != vk::Result::eSuccess) {
    Logger::error("Failed to create render image: {}",
                  vk::to_string(imageRes.result));
    return std::unexpected("Failed to create render image");
  }

  auto image = imageRes.value.first;
  auto allocation = imageRes.value.second;

  vk::ImageViewCreateInfo imageViewCreateInfo{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = format,
      .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor,
                           .baseMipLevel = 0,
                           .levelCount = 1,
                           .baseArrayLayer = 0,
                           .layerCount = 1}};

  VK_MAKE(imageView, device.createImageView(imageViewCreateInfo),
          "Failed to create image view for render image");

  vkh::AllocatedImage allocatedImage{.image = image,
                                     .view = std::move(imageView),
                                     .alloc = allocation,
                                     .extent = imageCreateInfo.extent,
                                     .format = format};

  return std::move(allocatedImage);
}

std::expected<ImGuiVkObjects, std::string>
setupImGui(GLFWwindow *window, const vk::raii::Instance &instance,
           const vk::raii::Device &device,
           const vk::raii::PhysicalDevice &physicalDevice,
           const vkh::Queue &graphicsQueue, const vk::Format swapchainFormat) {
  vk::DescriptorPoolSize pool_sizes[] = {
      {
          .type = vk::DescriptorType::eSampler,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eSampledImage,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eStorageImage,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eUniformTexelBuffer,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eStorageTexelBuffer,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eStorageBuffer,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eUniformBufferDynamic,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eStorageBufferDynamic,
          .descriptorCount = 1000,
      },
      {
          .type = vk::DescriptorType::eInputAttachment,
          .descriptorCount = 1000,
      }};

  vk::DescriptorPoolCreateInfo pool_info = {
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = 1000,
      .poolSizeCount = (uint32_t)std::size(pool_sizes),
      .pPoolSizes = pool_sizes,
  };

  VK_MAKE(imguiPool, device.createDescriptorPool(pool_info),
          "Failed to create ImGui descriptor pool");

  // 2: initialize imgui library

  // this initializes the core structures of imgui
  ImGui::CreateContext();

  // this initializes imgui for SDL
  ImGui_ImplGlfw_InitForVulkan(window, true);

  // this initializes imgui for Vulkan
  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.ApiVersion = VK_API_VERSION_1_4;
  init_info.Instance = *instance;
  init_info.PhysicalDevice = *physicalDevice;
  init_info.Device = *device;
  init_info.QueueFamily = graphicsQueue.index;
  init_info.Queue = **graphicsQueue.queue;
  init_info.DescriptorPool = *imguiPool;
  init_info.MinImageCount = 3;
  init_info.ImageCount = 3;
  init_info.UseDynamicRendering = true;

  VkPipelineRenderingCreateInfo info{};
  info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
  info.colorAttachmentCount = 1;

  auto swapchainFormatC = static_cast<VkFormat>(swapchainFormat);

  info.pColorAttachmentFormats = &swapchainFormatC;

  // dynamic rendering parameters for imgui to use
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo = info;

  ImGui_ImplVulkan_Init(&init_info);

  return ImGuiVkObjects{.descriptorPool = std::move(imguiPool)};
} // namespace engine::setup

} // namespace engine::setup
