#pragma once

#include "surface.h"
#include <vulkan/vulkan.h>

class Queue {
protected:
  VkQueue queue;

public:
  Queue(VkQueue queue) : queue(VkQueue(queue)) {}

  VkResult submit(VkSubmitInfo &submitInfo, VkFence fence = VK_NULL_HANDLE) {
    return vkQueueSubmit(queue, 1, &submitInfo, fence);
  }

  VkResult submit(std::vector<VkSubmitInfo> &submitInfo,
                  VkFence fence = VK_NULL_HANDLE) {
    return vkQueueSubmit(queue, submitInfo.size(), submitInfo.data(), fence);
  }
};

class QueueFamily {
  PhysicalDevice device;
  VkQueueFamilyProperties family;
  int index;

public:
  QueueFamily(PhysicalDevice &device, VkQueueFamilyProperties family, int index)
      : device(device), family(family), index(index) {}

  bool canPresentTo(Surface &surface) {
    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(*device, index, *surface,
                                         &presentSupport);
    return presentSupport;
  }

  bool hasGraphics() { return family.queueFlags & VK_QUEUE_GRAPHICS_BIT; }
};

class PresentQueue : Queue {
public:
  PresentQueue(VkQueue queue) : Queue(queue) {}
  PresentQueue(Queue &&other) : Queue(std::move(other)) {}
  VkResult present(VkPresentInfoKHR &presentInfo) {
    return vkQueuePresentKHR(queue, &presentInfo);
  }
};
