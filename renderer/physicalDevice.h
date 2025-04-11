#pragma once

#include <optional>
#include <print>
#include <vector>
#include <vulkan/vulkan.h>

#include "instance.h"

class PhysicalDevicePtr {
  VkPhysicalDevice device;

public:
  PhysicalDevicePtr(VkPhysicalDevice device) : device(device) {}

  PhysicalDevicePtr(const PhysicalDevicePtr &) = delete;
  PhysicalDevicePtr &operator=(const PhysicalDevicePtr &) = delete;

  PhysicalDevicePtr(PhysicalDevicePtr &&o) noexcept
      : device(std::move(o.device)) {
    std::println("Moving PhysicalDevicePtr");
    o.device = VK_NULL_HANDLE;
  }

  PhysicalDevicePtr &operator=(PhysicalDevicePtr &&o) noexcept {
    device = std::move(o.device);
    o.device = VK_NULL_HANDLE;
    printf("Assigning PhysicalDevicePtr\n");
    return *this;
  }

  VkPhysicalDevice &operator*() { return device; }
};

class PhysicalDevice {
  std::shared_ptr<PhysicalDevicePtr> device;
  std::optional<VkPhysicalDeviceProperties> properties;
  std::optional<VkPhysicalDeviceFeatures> features;

public:
  PhysicalDevice(VkPhysicalDevice device)
      : device(std::make_shared<PhysicalDevicePtr>(device)) {}

  static std::vector<PhysicalDevice> all(Instance &instance);

  VkPhysicalDeviceProperties getProperties();
  VkPhysicalDeviceFeatures getFeatures();

  VkPhysicalDevice &operator*() { return **device; }

  bool isDiscrete() {
    return getProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  }
};
