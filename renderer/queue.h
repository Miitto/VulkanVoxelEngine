#pragma once

#include "physicalDevice.h"
#include "surface.h"
#include <vulkan/vulkan.h>

class QueuePtr {
  VkQueue queue;

public:
  QueuePtr(VkQueue queue) : queue(queue) {}
  QueuePtr(const QueuePtr &) = delete;
  QueuePtr &operator=(const QueuePtr &) = delete;
  QueuePtr(QueuePtr &&o) noexcept : queue(std::move(o.queue)) {
    o.queue = VK_NULL_HANDLE;
  }
  QueuePtr &operator=(QueuePtr &&o) noexcept {
    queue = std::move(o.queue);
    o.queue = VK_NULL_HANDLE;
    return *this;
  }

  VkQueue &operator*() { return queue; }
};

class Queue {
  std::shared_ptr<QueuePtr> queue;

public:
  Queue(VkQueue queue) : queue(std::make_shared<QueuePtr>(queue)) {}
  static std::vector<VkQueueFamilyProperties> all(PhysicalDevice &device);
  static bool canPresent(PhysicalDevice &device,
                         VkQueueFamilyProperties &queueFamily, Surface &surface,
                         int index);
  static bool hasGraphics(VkQueueFamilyProperties &queueFamily);
};
