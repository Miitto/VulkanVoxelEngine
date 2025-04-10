#include "lib.h"
#include "physicalDevice.h"
#include "queue.h"
#include "swapChain.h"
#include "vkStructs/appInfo.h"
#include "vkStructs/deviceCreateInfo.h"
#include "vkStructs/deviceQueueCreationInfo.h"
#include "vkStructs/instanceCreateInfo.h"
#include "vkStructs/swapChainCreateInfo.h"
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
  SwapChainSupport swapChainSupport;
};

std::optional<DeviceReturn> findDevice(Instance &instance, Surface &surface) {
  auto physicalDevices = PhysicalDevice::all(instance);
  std::println("Found {} physical devices", physicalDevices.size());

  std::vector<uint32_t> ratings(physicalDevices.size(), 0);
  for (int i = 0; i < physicalDevices.size(); i++) {
    auto device = physicalDevices[i];
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
    auto swapChainSupport = SwapChainSupport(device, surface);

    if (swapChainSupport.formats.size() == 0 ||
        swapChainSupport.presentModes.size() == 0) {
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
  const PhysicalDevice *physicalDevice = nullptr;
  for (int i = 0; i < ratings.size(); i++) {
    if (ratings[i] > maxRating) {
      maxRating = ratings[i];
      physicalDevice = &physicalDevices[i];
    }
  }

  if (maxRating == 0) {
    std::cerr << "Failed to find a suitable physical device." << std::endl;
    return std::nullopt;
  }

  auto device = *physicalDevice;
  auto swapChainSupport = SwapChainSupport(device, surface);

  if (device.isDiscrete()) {
    std::println("Found discrete device: {}",
                 device.getProperties().deviceName);
  } else {
    std::println("Found integrated device: {}",
                 device.getProperties().deviceName);
  }

  DeviceReturn deviceReturn(device, swapChainSupport);

  return deviceReturn;
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

    VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                               static_cast<uint32_t>(height)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}
std::optional<App> App::create() {
  glfwInit();

  auto window = Window::create("Vulkan App", WIDTH, HEIGHT);
  if (!window.has_value()) {
    std::cerr << "Failed to create window." << std::endl;
    return std::nullopt;
  }

  auto instance = makeInstance();
  if (!instance.has_value()) {
    std::cerr << "Failed to create Vulkan instance." << std::endl;
    return std::nullopt;
  }

  auto surface = Surface::create(*instance, *window);
  if (!surface.has_value()) {
    std::cerr << "Failed to create surface." << std::endl;
    return std::nullopt;
  }

  auto deviceReturn = findDevice(*instance, *surface);
  if (!deviceReturn.has_value()) {
    std::cerr << "Failed to find a suitable physical device." << std::endl;
    return std::nullopt;
  }

  auto physicalDevice = deviceReturn->device;
  auto swapChainSupport = deviceReturn->swapChainSupport;
  auto format = chooseSwapSurfaceFormat(swapChainSupport.formats);
  auto presentMode = pickPresentMode(swapChainSupport.presentModes);
  auto extent = chooseSwapExtent(swapChainSupport.capabilities, *window);

  auto queueFamilies = Queue::all(physicalDevice);
  std::println("Found {} queue families", queueFamilies.size());

  std::optional<int> graphicsQueueFamilyIndex;
  std::optional<int> presentQueueFamilyIndex;

  for (int i = 0; i < queueFamilies.size(); i++) {
    bool graphics = Queue::hasGraphics(queueFamilies[i]);
    bool present =
        Queue::canPresent(physicalDevice, queueFamilies[i], *surface, i);

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

  auto logicalDevice =
      LogicalDevice::create(physicalDevice, deviceCreateInfoStruct);

  if (!logicalDevice.has_value()) {
    std::cerr << "Failed to create logical device." << std::endl;
    return std::nullopt;
  }

  auto graphicsQueue =
      logicalDevice->getQueue(graphicsQueueFamilyIndex.value(), 0);
  if (!graphicsQueue.has_value()) {
    std::cerr << "Failed to create graphics queue." << std::endl;
    return std::nullopt;
  }

  auto presentQueue =
      logicalDevice->getQueue(presentQueueFamilyIndex.value(), 0);
  if (!presentQueue.has_value()) {
    std::cerr << "Failed to create present queue." << std::endl;
    return std::nullopt;
  }

  SwapChainCreateInfoBuilder swapChainCreateInfo(swapChainSupport, *surface);
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

  auto swapChain = swapChainCreateInfo.build(*logicalDevice);
  if (!swapChain.has_value()) {
    std::cerr << "Failed to create swap chain." << std::endl;
    return std::nullopt;
  }

  App app(*window, *instance, *surface, *logicalDevice, *graphicsQueue,
          *presentQueue, *swapChain);

  return app;
}

void App::run() {
  std::println("Running app...");
  while (!window.shouldClose()) {
    glfwPollEvents();
  }
}

App::~App() {
  if (moved) {
    return;
  }

  swapChain.~SwapChain();
  device.~LogicalDevice();
  surface.~Surface();
  instance.~Instance();
  window.~Window();

  glfwTerminate();
}
