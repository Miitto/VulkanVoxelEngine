#pragma once

#include <vulkan/vulkan.h>

class DeviceQueueCreateInfoBuilder {
  float priority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo;

public:
  DeviceQueueCreateInfoBuilder(uint32_t queueIndex) {
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.pNext = nullptr;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = queueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority;
  }

  DeviceQueueCreateInfoBuilder &setQueueFamilyIndex(uint32_t queueIndex) {
    queueCreateInfo.queueFamilyIndex = queueIndex;
    return *this;
  }

  DeviceQueueCreateInfoBuilder &setQueueCount(uint32_t queueCount) {
    queueCreateInfo.queueCount = queueCount;
    return *this;
  }

  DeviceQueueCreateInfoBuilder &setQueuePriorities(float priority) {
    this->priority = priority;
    queueCreateInfo.pQueuePriorities = &priority;
    return *this;
  }

  VkDeviceQueueCreateInfo &build() { return queueCreateInfo; }
};
