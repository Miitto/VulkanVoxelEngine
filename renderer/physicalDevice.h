#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "instance.h"

class PhysicalDevice {
  VkPhysicalDevice device = VK_NULL_HANDLE;
  std::optional<VkPhysicalDeviceProperties> properties;
  std::optional<VkPhysicalDeviceFeatures> features;

  PhysicalDevice(VkPhysicalDevice device) : device(device) {}

public:
  // Move only, and clean up old device on move
  PhysicalDevice(const PhysicalDevice &) = delete;
  PhysicalDevice &operator=(const PhysicalDevice &) = delete;
  PhysicalDevice(PhysicalDevice &&o) noexcept
      : device(o.device), properties(o.properties), features(o.features) {
    o.device = nullptr;
  }
  PhysicalDevice &operator=(PhysicalDevice &&o) noexcept {
    device = o.device;
    properties = o.properties;
    features = o.features;
    o.device = nullptr;
    return *this;
  };

  static std::vector<PhysicalDevice> all(Instance &instance);

  VkPhysicalDeviceProperties getProperties();
  VkPhysicalDeviceFeatures getFeatures();

  VkPhysicalDevice &operator*() { return device; }

  bool isDiscrete() {
    return getProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  }
};
