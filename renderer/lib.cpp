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

  auto windowOpt = Window::create("Vulkan App", WIDTH, HEIGHT);
  if (!windowOpt.has_value()) {
    std::cerr << "Failed to create window." << std::endl;
    return std::nullopt;
  }
  std::println("Window created successfully.");

  auto appInfo = ApplicationInfoBuilder().build();
  auto instanceOpt = InstanceCreateInfoBuilder(appInfo)
                         .enableGLFWExtensions()
                         .enableValidationLayers()
                         .createInstance();
  if (!instanceOpt.has_value()) {
    std::cerr << "Failed to create Vulkan instance." << std::endl;
    return std::nullopt;
  }
  std::println("Vulkan instance created successfully.");

  auto window = std::move(*windowOpt);
  std::println("Window moved successfully.");
  windowOpt.reset();
  std::println("Window Reset successfully.");
  auto instance = std::move(*instanceOpt);
  instanceOpt.reset();

  auto physicalDevices = PhysicalDevice::all(instance);

  PhysicalDevice physicalDevice = std::move(*std::find_if(
      physicalDevices.begin(), physicalDevices.end(),
      [](PhysicalDevice &device) { return device.isDiscrete(); }));

  auto queueFamilies = Queue::all(physicalDevice);
  std::optional<int> graphicsQueueFamilyIndex;
  std::optional<int> presentQueueFamilyIndex;
  for (int i = 0; i < queueFamilies.size(); i++) {
    bool graphics = Queue::hasGraphics(queueFamilies[i]);
    bool present = Queue::canPresent(physicalDevice, queueFamilies[i], nullptr);

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

  auto logicalDeviceOpt =
      LogicalDevice::create(physicalDevice, deviceCreateInfoStruct);

  if (!logicalDeviceOpt.has_value()) {
    std::cerr << "Failed to create logical device." << std::endl;
    return std::nullopt;
  }

  auto logicalDevice = std::move(*logicalDeviceOpt);
  logicalDeviceOpt.reset();

  auto graphicsQueue =
      logicalDevice.getQueue(graphicsQueueFamilyIndex.value(), 0);

  auto presentQueue =
      logicalDevice.getQueue(presentQueueFamilyIndex.value(), 0);

  App app(std::move(window), std::move(instance), std::move(logicalDevice),
          std::move(graphicsQueue), std::move(presentQueue));
  std::println("App created successfully.");

  return app;
}

void App::run() {
  std::println("Running App");
  while (!window.shouldClose()) {
    glfwPollEvents();
  }
}

App::~App() {
  std::println("App Destructor");
  if (moved) {
    std::println("App already moved, skipping cleanup.");
    return;
  }
  device.~LogicalDevice();
  instance.~Instance();
  window.~Window();

  glfwTerminate();
}
