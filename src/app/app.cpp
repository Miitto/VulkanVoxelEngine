#include "app/app.hpp"
#include <algorithm>
#include <expected>

#include <GLFW/glfw3.h>
#include <memory>

#include "logger.hpp"

#include <engine/createInstance.hpp>
#include <engine/debug.hpp>
#include <engine/physicalDeviceSelector.hpp>
#include <engine/validators.hpp>

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

struct Queues {
  vk::raii::Queue graphicsQueue;
};

auto createLogicalDevice(const vk::raii::PhysicalDevice &physicalDevice)
    -> std::expected<std::tuple<vk::raii::Device, Queues>, std::string> {
  Logger::trace("Creating Logical Device");

  auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

  auto graphicsQueueFamilyIndex = static_cast<uint32_t>(std::distance(
      queueFamilyProperties.begin(),
      std::ranges::find_if(
          queueFamilyProperties, [](const vk::QueueFamilyProperties &props) {
            return bool(props.queueFlags & vk::QueueFlagBits::eGraphics);
          })));

  if (graphicsQueueFamilyIndex == queueFamilyProperties.size()) {
    return std::unexpected("No suitable graphics queue family found");
  }

  Logger::trace("Found Graphics Queue Family at index {}",
                graphicsQueueFamilyIndex);

  std::array<float, 1> queuePriority = {1.0f};

  auto queueCreateInfo =
      vk::DeviceQueueCreateInfo{.queueFamilyIndex = graphicsQueueFamilyIndex,
                                .queueCount = 1,
                                .pQueuePriorities = queuePriority.data()};

  vk::StructureChain<vk::PhysicalDeviceFeatures2,
                     vk::PhysicalDeviceVulkan13Features,
                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
      featursChain = {
          {}, {.dynamicRendering = true}, {.extendedDynamicState = true}};

  auto deviceCreateInfo = vk::DeviceCreateInfo{
      .pNext = &featursChain.get<vk::PhysicalDeviceFeatures2>(),
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
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
  auto &graphicsQueue = graphicsQueue_res.value();

  Queues queues{
      .graphicsQueue = std::move(graphicsQueue),
  };

  return std::make_tuple(std::move(device), queues);
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

  auto physicalDevice_res = pickPhysicalDevice(instance);
  if (!physicalDevice_res) {
    return std::unexpected(physicalDevice_res.error());
  }
  auto &physicalDevice = physicalDevice_res.value();

  auto logicalDevice_res = createLogicalDevice(physicalDevice);

  if (!logicalDevice_res) {
    return std::unexpected(logicalDevice_res.error());
  }

  [[maybe_unused]] auto &[logicalDevice, queues] = logicalDevice_res.value();

  Logger::trace("Creating App");
  App app(std::move(window), std::move(context), std::move(instance));

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
