#pragma once

#include <functional>
#include <optional>
#include <span>
#include <vulkan/vulkan_raii.hpp>

#include "logger.hpp"

namespace engine {
class PhysicalDeviceSelector {
public:
  struct DeviceSpecs {
    vk::raii::PhysicalDevice device;
    vk::PhysicalDeviceProperties properties;
    vk::PhysicalDeviceFeatures features;
    vk::PhysicalDeviceMemoryProperties memoryProperties;
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    std::vector<vk::ExtensionProperties> availableExtensions;

    uint32_t score = 0;
  };

private:
  std::vector<DeviceSpecs> physicalDevices;

  PhysicalDeviceSelector(std::vector<DeviceSpecs> &specs)
      : physicalDevices(std::move(specs)) {}

public:
  static auto create(vk::raii::Instance &instance)
      -> std::expected<PhysicalDeviceSelector, std::string> {
    auto devices = instance.enumeratePhysicalDevices();

    if (!devices) {
      return std::unexpected("Failed to get physical devices");
    }

    if (devices->empty()) {
      return std::unexpected("No physical devices found");
    }

    auto specs = std::vector<DeviceSpecs>();
    specs.reserve(devices->size());

    for (const auto &device : devices.value()) {
      DeviceSpecs spec{
          .device = std::move(device),
          .properties = device.getProperties(),
          .features = device.getFeatures(),
          .memoryProperties = device.getMemoryProperties(),
          .queueFamilyProperties = device.getQueueFamilyProperties(),
          .availableExtensions = device.enumerateDeviceExtensionProperties()};

      specs.push_back(std::move(spec));
    }

    PhysicalDeviceSelector selector(specs);

    return selector;
  }

  void requireExtensions(const std::span<const char *const> extensions) {
    Logger::debug("Requiring extensions:");
    for (const auto &ext : extensions) {
      Logger::debug("  - {}", ext);
    }

    Logger::trace("Filtering {} devices", physicalDevices.size());
    for (size_t i = physicalDevices.size() - 1; i != (~(size_t)0); --i) {
      Logger::trace("Checking device {} for required extensions", i);
      const auto &device = physicalDevices[i];

      for (const auto &ext : extensions) {
        if (std::ranges::find_if(
                device.availableExtensions,
                [&ext](const vk::ExtensionProperties &availableExt) {
                  return strcmp(availableExt.extensionName, ext) == 0;
                }) == device.availableExtensions.end()) {
          physicalDevices.erase(physicalDevices.begin() + i);
          break;
        }
      }
    }
  }

  void
  requireFeatures(const std::function<bool(const vk::PhysicalDeviceFeatures &)>
                      &featureCheck) {
    Logger::debug("Requiring features");
    for (size_t i = physicalDevices.size() - 1; i != (~(size_t)0); --i) {
      const auto &device = physicalDevices[i];

      if (!(featureCheck(device.features))) {
        physicalDevices.erase(physicalDevices.begin() + i);
      }
    }
  }

  void requireQueueFamily(vk::QueueFlagBits queueFlags) {
    Logger::debug("Requiring queue family with flags: {}",
                  vk::to_string(queueFlags));

    Logger::trace("Filtering {} devices", physicalDevices.size());
    for (size_t i = physicalDevices.size() - 1; i != (~(size_t)0); --i) {
      Logger::trace("Checking device {} for required queue family", i);
      const auto &device = physicalDevices[i];

      bool hasRequiredQueue = false;
      for (const auto &queueFamily : device.queueFamilyProperties) {
        if (queueFamily.queueFlags & queueFlags) {
          hasRequiredQueue = true;
          break;
        }
      }

      if (!hasRequiredQueue) {
        physicalDevices.erase(physicalDevices.begin() + i);
      }
    }
  }

  void requireMemoryType(uint32_t typeBits,
                         vk::MemoryPropertyFlags properties) {
    Logger::debug("Requiring memory type with type bits: {}, properties: {}",
                  typeBits, vk::to_string(properties));
    for (size_t i = physicalDevices.size() - 1; i != (~(size_t)0); --i) {
      const auto &device = physicalDevices[i];

      bool hasRequiredMemoryType = false;
      for (uint32_t j = 0; j < device.memoryProperties.memoryTypeCount; ++j) {
        if ((typeBits & (1 << j)) &&
            (device.memoryProperties.memoryTypes[j].propertyFlags &
             properties) == properties) {
          hasRequiredMemoryType = true;
          break;
        }
      }

      if (!hasRequiredMemoryType) {
        physicalDevices.erase(physicalDevices.begin() + i);
      }
    }
  }

  void requireVersion(uint32_t major, uint32_t minor, uint32_t patch) {
    Logger::debug("Requiring API version: {}.{}.{}", major, minor, patch);
    auto version = VK_MAKE_VERSION(major, minor, patch);
    for (size_t i = physicalDevices.size() - 1; i > 0; --i) {
      const auto &device = physicalDevices[i];

      if (device.properties.apiVersion < version) {
        physicalDevices.erase(physicalDevices.begin() + i);
      }
    }
  }

  void scoreDevices(
      const std::function<uint32_t(
          const engine::PhysicalDeviceSelector::DeviceSpecs &)> &scoreFn) {
    Logger::debug("Scoring devices");
    for (auto &device : physicalDevices) {
      device.score = scoreFn(device);
    }
  }

  void sortDevices() {
    Logger::debug("Sorting devices by score");
    std::sort(physicalDevices.begin(), physicalDevices.end(),
              [](DeviceSpecs &a, DeviceSpecs &b) { return a.score > b.score; });
  }

  auto select() -> std::vector<vk::raii::PhysicalDevice> {
    Logger::debug("Selecting devices");
    std::vector<vk::raii::PhysicalDevice> devices;
    devices.reserve(physicalDevices.size());
    for (const auto &spec : physicalDevices) {
      devices.push_back(spec.device);
    }
    return devices;
  }
};
} // namespace engine
