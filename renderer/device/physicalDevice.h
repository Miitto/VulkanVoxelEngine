#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

#include "instance.h"

class QueueFamily;

class PhysicalDevice {
  VkPhysicalDevice device;

public:
  PhysicalDevice(VkPhysicalDevice device) : device(device) {}

  VkPhysicalDevice &operator*() { return device; }

  static std::vector<PhysicalDevice> all(Instance &instance);

  VkPhysicalDeviceProperties getProperties() const;
  VkPhysicalDeviceFeatures getFeatures() const;
  std::vector<VkExtensionProperties> getExtensions() const;
  std::vector<QueueFamily> getQueues();

  struct MemoryProperties {
    uint32_t index;
    VkMemoryType memType;
  };

  VkPhysicalDeviceMemoryProperties getMemoryProperties() const;

  std::vector<char const *>
  findUnsupportedExtensions(std::vector<char const *> extensions) const;

  std::optional<PhysicalDevice::MemoryProperties>
  findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

  bool isDiscrete() {
    return getProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  }
};
