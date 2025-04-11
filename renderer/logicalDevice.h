#pragma once

#include "physicalDevice.h"
#include "queue.h"
#include <iostream>
#include <optional>
#include <vulkan/vulkan.h>

class LogicalDevicePtr {
  VkDevice device;

public:
  LogicalDevicePtr(VkDevice device) : device(device) {
    std::cout << "Creating logical device " << device << std::endl;
  }
  ~LogicalDevicePtr() {
    if (device != VK_NULL_HANDLE) {
      std::println("Destroying logical device");
      vkDestroyDevice(device, nullptr);
    }
  }

  LogicalDevicePtr(const LogicalDevicePtr &) = delete;
  LogicalDevicePtr &operator=(const LogicalDevicePtr &) = delete;

  LogicalDevicePtr(LogicalDevicePtr &&o) noexcept
      : device(std::move(o.device)) {
    std::println("Moving LogicalDevicePtr");
    o.device = VK_NULL_HANDLE;
  }

  LogicalDevicePtr &operator=(LogicalDevicePtr &&o) noexcept {
    std::println("Assigning LogicalDevicePtr");
    device = std::move(o.device);
    o.device = VK_NULL_HANDLE;
    return *this;
  }

  VkDevice &operator*() { return device; }
};

class LogicalDevice {
  std::shared_ptr<LogicalDevicePtr> device;

  LogicalDevice(VkDevice device)
      : device(std::make_shared<LogicalDevicePtr>(device)) {}

public:
  static std::optional<LogicalDevice> create(PhysicalDevice &physicalDevice,
                                             VkDeviceCreateInfo &createInfo);

  std::optional<Queue> getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex);

  VkDevice &operator*() { return **device; }
};
