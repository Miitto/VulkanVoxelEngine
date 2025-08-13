#include "app/app.hpp"
#include <algorithm>
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "logger.hpp"

#include <engine/createInstance.hpp>
#include <engine/debug.hpp>
#include <engine/physicalDeviceSelector.hpp>
#include <engine/swapchainConfig.hpp>
#include <engine/validators.hpp>
#include <engine/shaders.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char *WINDOW_TITLE = "Vulkan App";

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

const std::array<const char *, 4> requiredExtensions = {
    vk::KHRSwapchainExtensionName, vk::KHRSpirv14ExtensionName,
    vk::KHRSynchronization2ExtensionName,
    vk::KHRCreateRenderpass2ExtensionName};

auto pickPhysicalDevice(const vk::raii::Instance &instance)
    -> std::expected<vk::raii::PhysicalDevice, std::string> {
  auto physicalDeviceSelector_res =
      engine::PhysicalDeviceSelector::create(instance);

  if (!physicalDeviceSelector_res) {
    Logger::error("Failed to create Physical Device Selector: {}",
                  physicalDeviceSelector_res.error());
    return std::unexpected(physicalDeviceSelector_res.error());
  }

  auto &physicalDeviceSelector = physicalDeviceSelector_res.value();

  physicalDeviceSelector.requireExtensions(requiredExtensions);
  physicalDeviceSelector.requireVersion(1, 4, 0);
  physicalDeviceSelector.requireQueueFamily(vk::QueueFlagBits::eGraphics);

  physicalDeviceSelector.scoreDevices(
      [](const engine::PhysicalDeviceSelector::DeviceSpecs &spec) {
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
           physicalDevice.getSurfaceSupportKHR(index, surface);
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
      return physicalDevice.getSurfaceSupportKHR(index, surface);
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
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featursChain = {
          {}, {.dynamicRendering = true}, {.extendedDynamicState = true}};

  auto deviceCreateInfo = vk::DeviceCreateInfo{
      .pNext = &featursChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfo.size()),
      .pQueueCreateInfos = queueCreateInfo.data(),
      .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
      .ppEnabledExtensionNames = requiredExtensions.data()};

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

  App::Queues queues{.graphicsQueue = App::Queue {.index=graphicsQueueFamilyIndex, .queue=graphicsQueue},
                     .presentQueue = App::Queue {.index = presentQueueFamilyIndex, .queue=presentQueue}};

  return std::make_tuple(std::move(device), queues);
}

auto createSwapchain(const vk::raii::PhysicalDevice &physicalDevice,
                     const vk::raii::Device &device, GLFWwindow *window,
                     const vk::raii::SurfaceKHR &surface,
                     const App::Queues &queues) -> std::expected<std::tuple<App::SwapchainConfig, App::Swapchain>,
                                                     std::string> {
  auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);

  auto format = engine::chooseSwapSurfaceFormat(
      physicalDevice.getSurfaceFormatsKHR(surface));
  auto presentMode = engine::chooseSwapPresentMode(
      physicalDevice.getSurfacePresentModesKHR(surface));
  auto extent = engine::chooseSwapExtent(window, surfaceCapabilities);
  auto minImageCount =
      engine::minImageCount(surfaceCapabilities, MAX_FRAMES_IN_FLIGHT);
  auto desiredImageCount = engine::desiredImageCount(surfaceCapabilities);

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

  auto swapchain_res = device.createSwapchainKHR(swapchainCreateInfo);
  if (!swapchain_res) {
    Logger::error("Failed to create swapchain: {}",
                  vk::to_string(swapchain_res.error()));
    return std::unexpected("Failed to create swapchain");
  }

  auto &swapchain = swapchain_res.value();

  auto images = swapchain.getImages();

  vk::ImageViewCreateInfo imageViewCreateInfo{ .viewType = vk::ImageViewType::e2D, .format = swapchainConfig.format.format,
      .subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 } };

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

  App::Swapchain swapchain_objects {
      .swapchain = std::move(swapchain),
      .images = std::move(images),
      .imageViews = std::move(imageViews)};

  return std::make_tuple(swapchainConfig, std::move(swapchain_objects));
}

auto App::create() -> std::expected<App, std::string> {
  Logger::info("Creating GLFW window...");
  auto win_ptr = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE,
                                  nullptr, nullptr);
  auto window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
      win_ptr, [](GLFWwindow *window) {
        Logger::info("Destroying window");
        glfwDestroyWindow(window);
      });

  Logger::trace("Creating Context");
  auto context = vk::raii::Context();

  auto instance_res =
      engine::createInstance(context, "Voxel Engine", enableValidationLayers);
  if (!instance_res) {
    return std::unexpected(instance_res.error());
  }
  auto instance = std::move(instance_res.value());

  VkSurfaceKHR rawSurface;
  if (glfwCreateWindowSurface(*instance, window.get(), nullptr, &rawSurface) !=
      VK_SUCCESS) {
    Logger::error("Failed to create window surface");
    return std::unexpected("Failed to create window surface");
  }

  auto surface = vk::raii::SurfaceKHR(instance, rawSurface);

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

  auto swapchain_res =
      createSwapchain(physicalDevice, device, window.get(), surface, queues);

  if (!swapchain_res) {
    Logger::error("Failed to create swapchain: {}",
                  swapchain_res.error());
    return std::unexpected(swapchain_res.error());
  }

  auto &[swapchainConfig, swapchain] = swapchain_res.value();

  auto shader_res = engine::createShaderModule(
      device, "shaders/build/basic.spv");

  if (!shader_res) {
    Logger::error("Failed to create shader module: {}", shader_res.error());
    return std::unexpected(shader_res.error());
  }

  auto &shaderModule = shader_res.value();

  [[maybe_unused]]
  auto pipelineShaderStages =
      engine::createPipelineShaderStages(shaderModule);

  Logger::trace("Creating App");
  App app(window, context, instance, surface, device, queues, swapchainConfig,
          swapchain);

  if (enableValidationLayers) {
    Logger::trace("Creating Debug Messenger");
    auto debugMessenger = engine::makeDebugMessenger(app.instance, &app);
    if (!debugMessenger) {
      Logger::error("Failed to create Debug Messenger: {}",
                    vk::to_string(debugMessenger.error()));
    } else {
      app.setDebugMessenger(std::move(debugMessenger.value()));
    }
  } else {
    Logger::trace("Skipping Debug Messenger creation");
  }

  return app;
}
