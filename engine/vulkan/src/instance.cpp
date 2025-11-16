#include "vkh/instance.hpp"

#include <vkh/macros.hpp>

#include "vk-logger.hpp"
#include <GLFW/glfw3.h>

namespace vkh {
auto createInstance(vk::raii::Context &context, const char *appName,
                    const bool enableValidationLayers,
                    const std::span<const char *const> extraExtensions,
                    const std::span<const char *const> extraLayers)
    -> std::expected<vk::raii::Instance, std::string> {
  Logger::trace("Creating Instance");
  auto appInfo = vk::ApplicationInfo{
      .pApplicationName = appName,
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = vk::ApiVersion14,
  };

  auto glfwExtCount = 0u;
  auto glfwRequiredExtensions =
      glfwGetRequiredInstanceExtensions(&glfwExtCount);

  auto extensions = std::vector<const char *>(
      glfwRequiredExtensions, glfwRequiredExtensions + glfwExtCount);

  extensions.insert(extensions.end(), extraExtensions.begin(),
                    extraExtensions.end());

  auto layerNames = std::vector<const char *>{};

  layerNames.insert(layerNames.end(), extraLayers.begin(), extraLayers.end());

  if (enableValidationLayers) {
    printExtensions(context, spdlog::level::trace);
    layerNames.push_back("VK_LAYER_KHRONOS_validation");
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  auto missingExtensions = vkh::checkExtensions(context, extensions);
  if (!missingExtensions.empty()) {
    Logger::error("Missing required extensions:");
    for (const auto &ext : missingExtensions) {
      Logger::error("  - {}", ext);
    }
    return std::unexpected("Missing required Vulkan extensions");
  }

  auto missingLayers = vkh::checkLayers(context, layerNames);
  if (!missingLayers.empty()) {
    Logger::error("Missing required layers:");
    for (const auto &layer : missingLayers) {
      Logger::error("  - {}", layer);
    }
    return std::unexpected("Missing required Vulkan layers");
  }

  Logger::debug("Enabled Instance Extensions:");
  for (const auto &ext : extensions) {
    Logger::debug("  - {}", ext);
  }

  Logger::debug("Enabled Instance Layers:");
  for (const auto &layer : layerNames) {
    Logger::debug("  - {}", layer);
  }

  auto iCreateInfo = vk::InstanceCreateInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(layerNames.size()),
      .ppEnabledLayerNames = layerNames.data(),
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data()};
  VK_MAKE(instance, context.createInstance(iCreateInfo),
          "Failed to create Vulkan Instance");

  return std::move(instance);
}
} // namespace vkh
