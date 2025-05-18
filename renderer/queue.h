#pragma once

#include "structs/info/present.h"
#include "structs/info/submit.h"
#include "surface.h"
#include <cstdint>
#include <vulkan/vulkan.h>

class CommandBuffer;

class Queue {
protected:
  VkQueue queue;
  uint32_t familyIndex;

public:
  Queue(VkQueue queue, int familyIndex)
      : queue(VkQueue(queue)), familyIndex(familyIndex) {}

  VkResult submit(vk::info::Submit &submitInfo,
                  VkFence fence = VK_NULL_HANDLE) {
    return vkQueueSubmit(queue, 1, &submitInfo, fence);
  }

  VkResult submit(std::vector<vk::info::Submit> &submitInfo,
                  VkFence fence = VK_NULL_HANDLE) {
    return vkQueueSubmit(queue, static_cast<uint32_t>(submitInfo.size()),
                         submitInfo.data(), fence);
  }

  VkResult submit(CommandBuffer &cmdBuffer, VkFence fence = VK_NULL_HANDLE);

  VkResult waitIdle() { return vkQueueWaitIdle(queue); }

  uint32_t getFamilyIndex() const { return familyIndex; }
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
  PresentQueue(VkQueue queue, int familyIndex) : Queue(queue, familyIndex) {}
  PresentQueue(Queue &&other) : Queue(std::move(other)) {}
  VkResult present(vk::info::Present &presentInfo) {
    return vkQueuePresentKHR(queue, &presentInfo);
  }
};
