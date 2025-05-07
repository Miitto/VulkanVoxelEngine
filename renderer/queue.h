#pragma once

#include "surface.h"
#include <vulkan/vulkan.h>

class Queue {
  VkQueue queue;

public:
  Queue(VkQueue queue) : queue(VkQueue(queue)) {}
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
