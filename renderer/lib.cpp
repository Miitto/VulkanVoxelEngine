#include "lib.h"
#include "physicalDevice.h"
#include "queue.h"
#include "vkStructs/appInfo.h"
#include "vkStructs/deviceCreateInfo.h"
#include "vkStructs/deviceQueueCreationInfo.h"
#include "vkStructs/instanceCreateInfo.h"
#include <iostream>
#include <print>

const uint32_t WIDTH = 800, HEIGHT = 600;

std::optional<App> App::create() {
  glfwInit();

  auto window = Window::create("Vulkan App", WIDTH, HEIGHT);
  if (!window.has_value()) {
    std::cerr << "Failed to create window." << std::endl;
    return std::nullopt;
  }

  auto appInfo = ApplicationInfoBuilder().build();
  auto instance = InstanceCreateInfoBuilder(appInfo)
                      .enableGLFWExtensions()
                      .enableValidationLayers()
                      .createInstance();
  if (!instance.has_value()) {
    std::cerr << "Failed to create Vulkan instance." << std::endl;
    return std::nullopt;
  }

  auto surface = Surface::create(*instance, *window);
  if (!surface.has_value()) {
    std::cerr << "Failed to create surface." << std::endl;
    return std::nullopt;
  }

  auto physicalDevices = PhysicalDevice::all(*instance);
  std::println("Found {} physical devices", physicalDevices.size());

  auto find =
      std::find_if(physicalDevices.begin(), physicalDevices.end(),
                   [](PhysicalDevice &device) { return device.isDiscrete(); });

  if (find != physicalDevices.end()) {
    std::println("Found discrete GPU");
  } else {
    std::println("No discrete GPU found, using first available device");
  }

  PhysicalDevice physicalDevice = std::move(
      *(find != physicalDevices.end() ? find : physicalDevices.begin()));

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

  std::println("Found graphics queue family index: {}",
               graphicsQueueFamilyIndex.value());
  std::println("Found present queue family index: {}",
               presentQueueFamilyIndex.value());

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

  App app(std::move(*window), std::move(*instance), std::move(*surface),
          std::move(*logicalDevice), std::move(*graphicsQueue),
          std::move(*presentQueue));

  return app;
}

void App::run() {
  while (!window.shouldClose()) {
    glfwPollEvents();
  }
}

App::~App() {
  if (moved) {
    return;
  }
  device.~LogicalDevice();
  surface.~Surface();
  instance.~Instance();
  window.~Window();

  glfwTerminate();
}
