#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "instance.h"

class QueueFamily;

class PhysicalDevice {
  VkPhysicalDevice device;
  std::optional<VkPhysicalDeviceProperties> properties;
  std::optional<VkPhysicalDeviceFeatures> features;

public:
  PhysicalDevice(VkPhysicalDevice device) : device(device) {}

  VkPhysicalDevice &operator*() { return device; }

  static std::vector<PhysicalDevice> all(Instance &instance);

  VkPhysicalDeviceProperties getProperties();
  VkPhysicalDeviceFeatures getFeatures();
  std::vector<VkExtensionProperties> getExtensions() const;
  std::vector<char const *>
  findUnsupportedExtensions(std::vector<char const *> extensions) const;

  bool isDiscrete() {
    return getProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  }

  std::vector<QueueFamily> getQueues();
};
