#pragma once

#include "physicalDevice.h"
#include <unordered_map>
#include <vulkan/vulkan.h>

class Queue {
  VkQueue queue = VK_NULL_HANDLE;

public:
  // Move only, and clean up old device on move
  Queue(const Queue &) = delete;
  Queue &operator=(const Queue &) = delete;
  Queue(Queue &&o) noexcept : queue(o.queue) { o.queue = nullptr; }
  Queue &operator=(Queue &&o) noexcept {
    queue = o.queue;
    o.queue = nullptr;
    return *this;
  };

  Queue(VkQueue queue) : queue(queue) {}
  static std::vector<VkQueueFamilyProperties> all(PhysicalDevice &device);
  static bool canPresent(PhysicalDevice &device,
                         VkQueueFamilyProperties &queueFamily,
                         VkSurfaceKHR surface);
  static bool hasGraphics(VkQueueFamilyProperties &queueFamily);
};
