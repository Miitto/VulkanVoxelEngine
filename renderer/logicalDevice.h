#pragma once

#include "physicalDevice.h"
#include "queue.h"
#include <optional>
#include <vulkan/vulkan.h>

class LogicalDevice {
  VkDevice device = VK_NULL_HANDLE;

  LogicalDevice(VkDevice device) : device(device) {}

public:
  // Move only, and clean up old device on move
  LogicalDevice(const LogicalDevice &) = delete;
  LogicalDevice &operator=(const LogicalDevice &) = delete;
  LogicalDevice(LogicalDevice &&o) noexcept : device(o.device) {
    o.device = nullptr;
  }
  LogicalDevice &operator=(LogicalDevice &&o) noexcept {
    device = o.device;
    o.device = nullptr;
    return *this;
  };

  static std::optional<LogicalDevice> create(PhysicalDevice &physicalDevice,
                                             VkDeviceCreateInfo &createInfo);

  Queue getQueue(uint32_t queueFamilyIndex, uint32_t queueIndex) {
    VkQueue queue;
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &queue);
    return Queue(queue);
  }

  ~LogicalDevice();
};
