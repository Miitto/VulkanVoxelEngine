#include "vkh/validators.hpp"

#include "vk-logger.hpp"

namespace vkh {
void printExtensions(vk::raii::Context &context,
                     spdlog::level::level_enum logLevel) noexcept {
  auto extensions = context.enumerateInstanceExtensionProperties();

  Logger::log(logLevel, "Available Vulkan Extensions:");
  for (const auto &ext : extensions) {
    Logger::log(logLevel, "  - {}", ext.extensionName.data());
  }
}

std::vector<const char *>
checkExtensions(vk::raii::Context &context,
                std::span<const char *> extensions) noexcept {
  auto availableExtensions = context.enumerateInstanceExtensionProperties();

  std::vector<const char *> missingExtensions{};

  for (const auto &ext : extensions) {
    if (std::ranges::find_if(
            availableExtensions,
            [&ext](const vk::ExtensionProperties &availableExt) {
              return strcmp(availableExt.extensionName, ext) == 0;
            }) == availableExtensions.end()) {
      missingExtensions.push_back(ext);
    }
  }
  return missingExtensions;
}

std::vector<const char *> checkLayers(vk::raii::Context &context,
                                      std::span<const char *> layers) noexcept {
  auto availableLayers = context.enumerateInstanceLayerProperties();

  std::vector<const char *> missingLayers{};

  for (const auto &layer : layers) {
    if (std::ranges::find_if(
            availableLayers,
            [&layer](const vk::LayerProperties &availableLayer) {
              return strcmp(availableLayer.layerName, layer) == 0;
            }) == availableLayers.end()) {
      missingLayers.push_back(layer);
    }
  }
  return missingLayers;
}

} // namespace vkh