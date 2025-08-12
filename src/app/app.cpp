#include "app/app.hpp"
#include <algorithm>
#include <expected>

#include <GLFW/glfw3.h>
#include <engine/util/error.hpp>
#include <memory>

#include "logger.hpp"

#include <engine/validators.hpp>
#include <engine/debug.hpp>
#include <engine/createInstance.hpp>
#include <engine/physicalDeviceSelector.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char *WINDOW_TITLE = "Vulkan App";

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

auto App::create() -> std::expected<App, std::string> {
  Logger::info("Creating GLFW window...");
  auto win_ptr = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
  auto window = std::unique_ptr<GLFWwindow, void (*)(GLFWwindow *)>(
      win_ptr, [](GLFWwindow *window) {
        Logger::info("Destroying window");
        glfwDestroyWindow(window);
      });

  Logger::trace("Creating Context");
  auto context = vk::raii::Context();

  auto instance_res = engine::createInstance(
      context, "Voxel Engine", enableValidationLayers);
  if (!instance_res) {
    return std::unexpected(instance_res.error());
  }
  auto instance = std::move(instance_res.value());

  auto physicalDeviceSelector_res =
      engine::PhysicalDeviceSelector::create(instance);

  if (!physicalDeviceSelector_res) {
    Logger::error("Failed to create Physical Device Selector: {}",
                  physicalDeviceSelector_res.error());
    return std::unexpected(physicalDeviceSelector_res.error());
  }

  auto &physicalDeviceSelector =
      physicalDeviceSelector_res.value();

  const std::vector<const char *> requiredExtensions = {
      vk::KHRSwapchainExtensionName,
      vk::KHRSpirv14ExtensionName,
      vk::KHRSynchronization2ExtensionName,
      vk::KHRCreateRenderpass2ExtensionName};

  physicalDeviceSelector.requireExtensions(requiredExtensions);
  physicalDeviceSelector.requireVersion(1, 4, 0);
  physicalDeviceSelector.requireQueueFamily(vk::QueueFlagBits::eGraphics);

  physicalDeviceSelector.scoreDevices(
      [](const engine::PhysicalDeviceSelector::DeviceSpecs &spec) {
        uint32_t score = 0;
        if (spec.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
          score += 1000;
        }

        return score;
      });

  physicalDeviceSelector.sortDevices();

  auto physicalDevices =
      physicalDeviceSelector.select();

  if (physicalDevices.empty()) {
    Logger::error("No suitable physical devices found");
    return std::unexpected("No suitable physical devices found");
  }

  Logger::info("Found {} suitable physical devices",
              physicalDevices.size());
  auto physicalDevice = physicalDevices.front();

  Logger::info("Using physical device: {}",
              physicalDevice.getProperties().deviceName.data());

  Logger::trace("Creating App");
  App app(std::move(window), std::move(context), std::move(instance));

  if (enableValidationLayers) {
    Logger::trace("Creating Debug Messenger");
    auto debugMessenger =
        engine::makeDebugMessenger(app.instance, &app);
    if (!debugMessenger) {
      Logger::error("Failed to create Debug Messenger: {}",
                    Result(debugMessenger.error()));
    } else {
      app.setDebugMessenger(std::move(debugMessenger.value()));
    }
  } else {
    Logger::trace("Skipping Debug Messenger creation");
  }

  return app;
}


